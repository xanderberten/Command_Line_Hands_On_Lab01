#include <iostream>
#include <fstream>
//RapidJson
#include <algorithm>
#include <string>
#include <vector>
#include "fstream"
#include "rapidjson.h"
#include "document.h"
#include "istreamwrapper.h"
#include "filereadstream.h"

struct float3
{
	float x;
	float y;
	float z;
};

enum class BlockType
{
	dirt = 0,
	stone = 1,
	wood = 2,
	glass = 3
};

enum class FacesToRender
{
	top,
	bottom,
	front,
	back,
	left,
	right
};

struct Block
{
	float3 position;
	BlockType type;
	bool isOpaque;
	std::vector<FacesToRender> facesToRender;
};

void SetInputAndOutputLocations(int argc, char** argv, std::string& input, std::string& output)
{
	//TODO: do check for filenames and early exits
	if (argc == 3 || argc == 5)
	{
		if (argv[1])
		{
			std::string value{ argv[1] };
			if (value._Equal("-i"))
			{
				if (argv[2])
				{
					input = argv[2];
					output = "scene.obj";
				}
			}
		}
	}

	if (argc == 5)
	{
		if (argv[3])
		{
			std::string value{ argv[3] };
			if (value._Equal("-o"))
			{
				if (argv[4])
				{
					output = argv[4];
				}
			}
		}
	}
}

std::vector<Block> GetBlocksOfJson(const rapidjson::Value& jsonArray)
{
	std::vector<Block> blocks{};

	size_t index{};
	for (; index < jsonArray.Size(); index++)
	{
		//Selects Block Type And Positions Array
		const rapidjson::Value& positionArray{ jsonArray[index]["positions"] };

		const bool isOpaque{ jsonArray[index]["opaque"].GetBool() };

		for (size_t i{}; i < positionArray.Size(); ++i)
		{
			//Selects 1 position off a block
			const rapidjson::Value& blockPosition = positionArray[i];

			//Get array of opaque and non opaque blocks
			blocks.emplace_back(Block{ float3{ blockPosition[0].GetFloat(), blockPosition[1].GetFloat(), blockPosition[2].GetFloat()},BlockType{BlockType(index)}, isOpaque });
		}
	}
	blocks.shrink_to_fit();
	return blocks;
}

//Checks if there is a neighbor and
//Sets a flag what faces  to render
void hasOpaqueNeighbors(Block& block, const std::vector<Block>& blocks)
{
	//Note: This will only work when the blocks are are on a uniform grid

	//if (!block.isOpaque)
	{
		//Check X Direction
		if (std::find_if(blocks.begin(), blocks.end(), [&](const Block& otherBlock)
			{
				//if (!otherBlock.isOpaque) return true;
				return(
					otherBlock.position.x == block.position.x + 1
					&& otherBlock.position.y == block.position.y
					&& otherBlock.position.z == block.position.z);
		//This is the actual if() check
		//If the lambda function returns false for all elements in the vector, then std::find_if returns an iterator equal to blocks.end().
		//So this will evaluate to false if not all elements return false (if there are no neighbors)
		//In other words this evaluates to false if there is a neighbor
			}) == blocks.end())
		{
			block.facesToRender.emplace_back(FacesToRender::right);
		}


			if (std::find_if(blocks.begin(), blocks.end(), [&](const Block& otherBlock)
				{
					//if (otherBlock.isOpaque) return false;
					return(
						otherBlock.position.x == block.position.x - 1
						&& otherBlock.position.y == block.position.y
						&& otherBlock.position.z == block.position.z);
				}) == blocks.end())
			{
				block.facesToRender.emplace_back(FacesToRender::left);
			}

				//Check Y Direction
				if (std::find_if(blocks.begin(), blocks.end(), [&](const Block& otherBlock)
					{
						//if (otherBlock.isOpaque) return false;
						return(
							otherBlock.position.x == block.position.x
							&& otherBlock.position.y == block.position.y + 1
							&& otherBlock.position.z == block.position.z);
					}) == blocks.end())
				{
					block.facesToRender.emplace_back(FacesToRender::front);
				}

					if (std::find_if(blocks.begin(), blocks.end(), [&](const Block& otherBlock)
						{
							//if (otherBlock.isOpaque) return false;
							return(
								otherBlock.position.x == block.position.x
								&& otherBlock.position.y == block.position.y - 1
								&& otherBlock.position.z == block.position.z);
						}) == blocks.end())
					{
						block.facesToRender.emplace_back(FacesToRender::back);
					}

						//Check Z Direction
						if (std::find_if(blocks.begin(), blocks.end(), [&](const Block& otherBlock)
							{
								//if (otherBlock.isOpaque) return false;
								return(
									otherBlock.position.x == block.position.x
									&& otherBlock.position.y == block.position.y
									&& otherBlock.position.z == block.position.z + 1);
							}) == blocks.end())
						{
							block.facesToRender.emplace_back(FacesToRender::top);
						}

							if (std::find_if(blocks.begin(), blocks.end(), [&](const Block& otherBlock)
								{
									//if (otherBlock.isOpaque) return false;
									return(
										otherBlock.position.x == block.position.x
										&& otherBlock.position.y == block.position.y
										&& otherBlock.position.z == block.position.z - 1);
								}) == blocks.end())
							{
								block.facesToRender.emplace_back(FacesToRender::bottom);
							}
	}

}

int main(int argc, char** argv)
{
	std::string jsonFileLocation{};
	std::string outputLocation{};
	std::vector<Block> blocks{};

	SetInputAndOutputLocations(argc, argv, jsonFileLocation, outputLocation);

	//Read .json File
	if (std::ifstream is{ jsonFileLocation })
	{
		std::cout << ".json file to read from found\n";

		rapidjson::Document jsonDoc;

		rapidjson::IStreamWrapper isw{ is };
		jsonDoc.ParseStream(isw);

		const rapidjson::Value& parentArray = jsonDoc;
		blocks = GetBlocksOfJson(parentArray);
	}
	else
	{
		std::cout << ".json file not found\n";
		return -2; // file not found.
	}

	//Checks if there is a neighbor and
	//Sets a flag what faces not to render
	for (size_t i{}; i < blocks.size(); ++i)
	{
		hasOpaqueNeighbors(blocks[i], blocks);
	}

	FILE* pOutputFile = nullptr;

	//TODO:: Fix output file
	_wfopen_s(&pOutputFile, L"scene.obj", L"w+,ccs=UTF-8");

	if (pOutputFile != nullptr)
	{
		const wchar_t* text = L"# is the symbol for partial derivative.\n";
		fwrite(text, wcslen(text) * sizeof(wchar_t), 1, pOutputFile);

		int faceAddition{};
		for (size_t i{}; i < blocks.size(); i++)
		{
			//Vertices Positions
			//fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 0.5f);//Left		Back	Bottom
			//fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z + 0.5f);//Left		Back	Top		
			//fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z - 0.5f);//Left		Front	Bottom	
			//fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z + 0.5f);//Left		Front	Top
			//fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 0.5f);//Right		Back	Bottom
			//fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z + 0.5f);//Right		Back	Top
			//fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z - 0.5f);//Right		Front	Bottom
			//fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z + 0.5f);//Right		Front	Top

			int amountOfFaces{};
			for (size_t face{}; face < blocks[i].facesToRender.size(); face++)
			{
				switch (blocks[i].facesToRender[face])
				{
				case FacesToRender::top:
				{
					fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z + 0.5f);//Left		Back	Top
					fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z + 0.5f);//Left		Front	Top
					fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z + 0.5f);//Right		Back	Top
					fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z + 0.5f);//Right		Front	Top

					fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z + 0.5f);//Up
					//Up
					fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 2 + faceAddition, 7 + faceAddition, 2 + faceAddition, 5 + faceAddition, 2 + faceAddition);
					fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 2 + faceAddition, 3 + faceAddition, 2 + faceAddition, 7 + faceAddition, 2 + faceAddition);

					amountOfFaces += 2;
					break;
				}
				case FacesToRender::bottom:
				{
					fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 0.5f);//Left		Back	Bottom
					fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z - 0.5f);//Left		Front	Bottom
					fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 0.5f);//Right		Back	Bottom
					fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z - 0.5f);//Right		Front	Bottom

					fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 1.5f);//Down
					//Down
					fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 6 + faceAddition, 4 + faceAddition, 6 + faceAddition, 3 + faceAddition, 6 + faceAddition);
					fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 6 + faceAddition, 2 + faceAddition, 6 + faceAddition, 4 + faceAddition, 6 + faceAddition);

					amountOfFaces += 2;
					break;
				}
				case FacesToRender::front:
				{
					fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y + 1.5f, blocks[i].position.z - 0.5f);//Front

					if (std::find_if(blocks[i].facesToRender.begin(), blocks[i].facesToRender.end(), [&](const FacesToRender otherFace)
						{
							return otherFace == FacesToRender::top;
						}) != blocks[i].facesToRender.end())
					{
						//When there is no top face that needs to be rendered
						fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z + 0.5f);//Left		Front	Top
						fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z + 0.5f);//Right		Front	Top

						faceAddition -= 1;
					}

						if (std::find_if(blocks[i].facesToRender.begin(), blocks[i].facesToRender.end(), [&](const FacesToRender otherFace)
							{
								return otherFace == FacesToRender::bottom;
							}) != blocks[i].facesToRender.end())
						{
							//When there is no bottom face that needs to be rendered
							fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z - 0.5f);//Left		Front	Bottom	
							fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y + 0.5f, blocks[i].position.z - 0.5f);//Right		Front	Bottom
							faceAddition -= 1;
						}

							//Front
							fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 3 + faceAddition, 3 + faceAddition, 8 + faceAddition, 3 + faceAddition, 7 + faceAddition, 3 + faceAddition);
							fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 3 + faceAddition, 3 + faceAddition, 4 + faceAddition, 3 + faceAddition, 8 + faceAddition, 3 + faceAddition);

							amountOfFaces += 2;
							break;
				}
				case FacesToRender::back:
				{
					if (std::find_if(blocks[i].facesToRender.begin(), blocks[i].facesToRender.end(), [&](const FacesToRender otherFace)
						{
							return otherFace == FacesToRender::top;
						}) != blocks[i].facesToRender.end())
					{
						//When there is no top face that needs to be rendered
						fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z + 0.5f);//Left		Back	Top
						fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z + 0.5f);//Right		Back	Top
					}

						if (std::find_if(blocks[i].facesToRender.begin(), blocks[i].facesToRender.end(), [&](const FacesToRender otherFace)
							{
								return otherFace == FacesToRender::bottom;
							}) != blocks[i].facesToRender.end())
						{
							//When there is no bottom face that needs to be rendered
							fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 0.5f);//Left		Back	Bottom
							fwprintf_s(pOutputFile, L"v %.4f %.4f %.4f\n", blocks[i].position.x + 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 0.5f);//Right		Back	Bottom
						}

							fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 1.5f, blocks[i].position.z - 0.5f);//Back

							//Back
							fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 5 + faceAddition, 5 + faceAddition, 7 + faceAddition, 5 + faceAddition, 8 + faceAddition, 5 + faceAddition);
							fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 5 + faceAddition, 5 + faceAddition, 8 + faceAddition, 5 + faceAddition, 6 + faceAddition, 5 + faceAddition);

							amountOfFaces += 2;
							break;
				}
				case FacesToRender::left:
				{

				}
				case FacesToRender::right: {break; }

				}

				////Normals
				//fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z + 0.5f);//Up
				//fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 1.5f);//Down
				//fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y + 1.5f, blocks[i].position.z - 0.5f);//Front
				//fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x - 0.5f, blocks[i].position.y - 1.5f, blocks[i].position.z - 0.5f);//Back
				//fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x + 1.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 0.5f);//Right
				//fwprintf_s(pOutputFile, L"vn %.4f %.4f %.4f\n", blocks[i].position.x - 1.5f, blocks[i].position.y - 0.5f, blocks[i].position.z - 0.5f);//Left



				////Up
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 2 + faceAddition, 7 + faceAddition, 2 + faceAddition, 5 + faceAddition, 2 + faceAddition);
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 2 + faceAddition, 3 + faceAddition, 2 + faceAddition, 7 + faceAddition, 2 + faceAddition);
				////Down
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 6 + faceAddition, 4 + faceAddition, 6 + faceAddition, 3 + faceAddition, 6 + faceAddition);
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 6 + faceAddition, 2 + faceAddition, 6 + faceAddition, 4 + faceAddition, 6 + faceAddition);
				////Front
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 3 + faceAddition, 3 + faceAddition, 8 + faceAddition, 3 + faceAddition, 7 + faceAddition, 3 + faceAddition);
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 3 + faceAddition, 3 + faceAddition, 4 + faceAddition, 3 + faceAddition, 8 + faceAddition, 3 + faceAddition);
				////Back
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 5 + faceAddition, 5 + faceAddition, 7 + faceAddition, 5 + faceAddition, 8 + faceAddition, 5 + faceAddition);
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 5 + faceAddition, 5 + faceAddition, 8 + faceAddition, 5 + faceAddition, 6 + faceAddition, 5 + faceAddition);
				////Right
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 4 + faceAddition, 5 + faceAddition, 4 + faceAddition, 6 + faceAddition, 4 + faceAddition);
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 1 + faceAddition, 4 + faceAddition, 6 + faceAddition, 4 + faceAddition, 2 + faceAddition, 4 + faceAddition);
				////Left
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 2 + faceAddition, 1 + faceAddition, 6 + faceAddition, 1 + faceAddition, 8 + faceAddition, 1 + faceAddition);
				//fwprintf_s(pOutputFile, L"f %d//%d %d//%d %d//%d\n", 2 + faceAddition, 1 + faceAddition, 8 + faceAddition, 1 + faceAddition, 4 + faceAddition, 1 + faceAddition);

				//ammountOfFaces += 8;
				faceAddition += amountOfFaces;
			}
		}

		fclose(pOutputFile);
		return 0;
	}
}