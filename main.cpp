#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

#include "HAPI\HAPI.h"
#include "HAPI\HoudiniApi.h"

using namespace std;


int main()
{

	// load HAPI library
	auto hapiLibHandle = LoadLibrary(L"D:/Houdini/bin/libHAPIL.dll");
	if (hapiLibHandle == NULL)
	{
		cout << "Failed to load HAPI library" << endl;
		return 1;
	}

	typedef HAPI_Result(*AddAttributeFunPtr)(const HAPI_Session* session, HAPI_NodeId node_id, HAPI_PartId part_id, const char* name, HAPI_AttributeInfo* attr_info);

	AddAttributeFunPtr AddAttribute = (AddAttributeFunPtr)GetProcAddress(hapiLibHandle, "HAPI_AddAttribute");

	std::cout << "AddAttribute: " << AddAttribute << std::endl;

	FHoudiniApi::InitializeHAPI(hapiLibHandle);

	HAPI_Session session;
	FHoudiniApi::CreateInProcessSession(&session);

	HAPI_CookOptions cookOptions;
	HAPI_Result result = FHoudiniApi::Initialize(
		&session,
		&cookOptions,
		false,
		-1,
		"",
		"",
		"",
		"",
		""
	);
	
	if (result != HAPI_Result::HAPI_RESULT_SUCCESS)
	{
		cout << "Failed to initial session" << endl;
	}

	HAPI_NodeId objNode = -1;
	FHoudiniApi::GetManagerNodeId(&session, HAPI_NodeType::HAPI_NODETYPE_OBJ, &objNode);

	HAPI_NodeId geoNode = -1;
	FHoudiniApi::CreateNode(&session, objNode, "geo", "MyGeo", true, &geoNode);

	HAPI_NodeId testNode = -1;
	FHoudiniApi::CreateNode(&session, geoNode, "platonic", "MyTest", true, &testNode);

	FHoudiniApi::SetParmIntValue(&session, testNode, "type", 0, 3);

	FHoudiniApi::SaveGeoToFile(&session, testNode, "G:\\C++\\Houdini\\TestGeo.bgeo");

	HAPI_AssetLibraryId assetLibId = -1;

	auto LoadHDAFile = [&result, &session, &assetLibId](const char* InAssetFileName)
		{
			std::string AssetFileName = InAssetFileName;
			result = FHoudiniApi::LoadAssetLibraryFromFile(
				&session,
				AssetFileName.c_str(),
				true,
				&assetLibId
			);
		};


	// 加载 hda 资产
	LoadHDAFile("C:/Users/32914/Downloads/SideFX__fence.otl");

	// 获取 HDA 资产名称
	std::vector<HAPI_StringHandle> assetNames;
	int assetCount = 0;
	result = FHoudiniApi::GetAvailableAssetCount(&session, assetLibId, &assetCount);
	if (result != HAPI_RESULT_SUCCESS)
	{
		std::cout << "Failed to get available asset count" << std::endl;
	}

	assetNames.resize(assetCount);

	result = FHoudiniApi::GetAvailableAssets(&session, assetLibId, &assetNames[0], assetCount);
	if (result != HAPI_RESULT_SUCCESS)
	{
		std::cout << "Failed to get available assets name" << std::endl;
	}


	// 获取 HDA Asset name
	std::string assetNamesStr;
	
	{
		int nameLength = 0;
		result = FHoudiniApi::GetStringBufLength(&session, assetNames[0], &nameLength);
		if (result != nameLength && nameLength <= 0)
		{
			cout << "get name length failed" << endl;
		}

		std::vector<char> nameBuffer(nameLength, '\0');
		result = FHoudiniApi::GetString(&session, assetNames[0], &nameBuffer[0], nameLength);
		if (result != HAPI_RESULT_SUCCESS)
		{
			cout << "get name failed" << endl;
		}

		assetNamesStr = std::string(nameBuffer.begin(), nameBuffer.end());
	}
	
	
	int parmCount = 0;
	int DefaultIntValueCount = 0;
	int DefaultFloatValueCount = 0;
	int DefaultStringValueCount = 0;
	int DefaultChoiceValueCount = 0;

	result = FHoudiniApi::GetAssetDefinitionParmCounts(
		&session,
		assetLibId,
		assetNamesStr.c_str(),
		&parmCount,
		&DefaultIntValueCount,
		&DefaultFloatValueCount,
		&DefaultStringValueCount,
		&DefaultChoiceValueCount
	);


	
	
	FHoudiniApi::Cleanup(&session);
	FHoudiniApi::CloseSession(&session);

	



	FreeLibrary(hapiLibHandle);
	return 0;
}