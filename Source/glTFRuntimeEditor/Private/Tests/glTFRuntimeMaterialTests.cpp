#if WITH_DEV_AUTOMATION_TESTS
#include "glTFRuntimeEditor.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"

namespace
{
	// One 1x1 PNG: texture 2 has sampler 1, the others sampler 0; material N shows texture N.
	const char* const glTFRuntimeMaterialTests_OneImage = R"gltf({
		"asset": { "version": "2.0" },
		"images": [ { "uri": "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP4z8DwHwAFAAH/VscvDQAAAABJRU5ErkJggg==" } ],
		"samplers": [ { "wrapS": 33071 }, { "wrapS": 33648 } ],
		"textures": [
			{ "source": 0, "sampler": 0 },
			{ "source": 0, "sampler": 0 },
			{ "source": 0, "sampler": 1 },
			{ "source": 0, "sampler": 0 }
		],
		"materials": [
			{ "pbrMetallicRoughness": { "baseColorTexture": { "index": 0 } } },
			{ "pbrMetallicRoughness": { "baseColorTexture": { "index": 1 } } },
			{ "pbrMetallicRoughness": { "baseColorTexture": { "index": 2 } } },
			{ "normalTexture": { "index": 3 } }
		]
	})gltf";

	UTexture2D* glTFRuntimeMaterialTests_LoadTexture(UglTFRuntimeAsset& Asset, const FglTFRuntimeMaterialsConfig& Config, const int32 MaterialIndex, const FName ParameterName)
	{
		UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Asset.LoadMaterial(MaterialIndex, Config, /* bUseVertexColors */ false));
		return Material ? Cast<UTexture2D>(Material->K2_GetTextureParameterValue(ParameterName)) : nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FglTFRuntimeTests_Material_OneTexturePerImage, "glTFRuntime.UnitTests.Material.OneTexturePerImage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FglTFRuntimeTests_Material_OneTexturePerImage::RunTest(const FString& Parameters)
{
	FglTFRuntimeConfig LoaderConfig;
	UglTFRuntimeAsset* Unshared = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromString(glTFRuntimeMaterialTests_OneImage, LoaderConfig);
	UglTFRuntimeAsset* Shared = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromString(glTFRuntimeMaterialTests_OneImage, LoaderConfig);
	if (!TestTrue("The assets load", Unshared && Shared))
	{
		return false;
	}

	FglTFRuntimeMaterialsConfig ShareConfig;
	ShareConfig.bShareTexturesOfOneImage = true;

	UTexture2D* First = glTFRuntimeMaterialTests_LoadTexture(*Unshared, FglTFRuntimeMaterialsConfig(), 0, "baseColorTexture");
	UTexture2D* Second = glTFRuntimeMaterialTests_LoadTexture(*Unshared, FglTFRuntimeMaterialsConfig(), 1, "baseColorTexture");
	UTexture2D* BaseColor = glTFRuntimeMaterialTests_LoadTexture(*Shared, ShareConfig, 0, "baseColorTexture");
	UTexture2D* SameSampler = glTFRuntimeMaterialTests_LoadTexture(*Shared, ShareConfig, 1, "baseColorTexture");
	UTexture2D* OtherSampler = glTFRuntimeMaterialTests_LoadTexture(*Shared, ShareConfig, 2, "baseColorTexture");
	UTexture2D* Normal = glTFRuntimeMaterialTests_LoadTexture(*Shared, ShareConfig, 3, "normalTexture");
	if (!TestTrue("Every material shows a texture", First && Second && BaseColor && SameSampler && OtherSampler && Normal))
	{
		return false;
	}

	TestTrue("By default each entry makes its own texture", First != Second);
	TestTrue("One image and one sampler make one texture", BaseColor == SameSampler);
	TestTrue("Another sampler makes another texture", BaseColor != OtherSampler);
	TestTrue("Another role makes another texture", BaseColor != Normal);

	return true;
}

#endif
