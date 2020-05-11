// shp_io_shplib.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

// reference: 原文链接：https://blog.csdn.net/feihongchen/article/details/105462156

#include <iostream>
#include <vector>
#include <string>

#include <shapefil.h>

#define MAX_PATH  256

struct Point3d
{
	double x, y, z;
	Point3d() {
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}
	Point3d(double _x, double _y, double _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
};

bool writeShapeFile(const std::string& filename, const std::vector<std::vector<Point3d>>& polylines)
{
	bool o_flag = false;

	// create filename.shp, filename.dbf
	SHPHandle hShp = SHPCreate(std::string(filename + ".shp").c_str(), SHPT_ARCZ);
	DBFHandle hDbf = DBFCreate(std::string(filename + ".dbf").c_str());

	int nlines = polylines.size();
	if (0 == nlines)
	{
		return o_flag;
	}
	SHPObject* shpObject;

	for (int i = 0; i < nlines; ++i)
	{
		// field index
		int field_idx = 0; 

		int nVertices = polylines[i].size();
		double* padfX = new double[nVertices];
		double* padfY = new double[nVertices];
		double* padfZ = new double[nVertices];
		double* padfm = new double[nVertices];

		for (int j = 0; j < nVertices; ++j)
		{
			padfX[j] = polylines[i][j].x;
			padfY[j] = polylines[i][j].y;
			padfZ[j] = polylines[i][j].z;
			padfm[j] = i;
		}

		shpObject = SHPCreateObject(SHPT_ARCZ, -1, 0, NULL, NULL, nVertices, padfX, padfY, padfZ, padfm);
		SHPWriteObject(hShp, -1, shpObject);
		SHPDestroyObject(shpObject);
		
		delete[] padfX;
		delete[] padfY;
		delete[] padfZ;
		delete[] padfm;
		padfX = NULL;
		padfY = NULL;
		padfZ = NULL;
		padfm = NULL;

		// create .dbf attributes
		DBFAddField(hDbf, "ID", FTInteger, 10, 0);
		DBFAddField(hDbf, "Name", FTString, 10, 0);
		DBFAddField(hDbf, "Length", FTDouble, 32, 0);

		// dbf record
		int record_idx = DBFGetRecordCount(hDbf);

		DBFWriteIntegerAttribute(hDbf, record_idx, field_idx++, 1001);
		DBFWriteStringAttribute(hDbf, record_idx, field_idx++, "polyline");
		DBFWriteDoubleAttribute(hDbf, record_idx, field_idx++, 10.15);
	}
	
	DBFClose(hDbf);
	SHPClose(hShp);
	o_flag = true;

	return o_flag;
}

bool readPolylines(const std::string& filename, std::vector<std::vector<Point3d>>& polylines) {
	bool in_flag = false;

	SHPHandle hSHP;
	DBFHandle hDBF;
	int nShapeType, nEntities;
	double adfMinBound[4], adfMaxBound[4];

	hSHP = SHPOpen(filename.c_str(), "r");
	if (hSHP == NULL)
	{
		return in_flag;
	}

	char szDBF[MAX_PATH + 1];

	strcpy_s(szDBF, filename.c_str());
	szDBF[strlen(szDBF) - 3] = '\0';
	strcat_s(szDBF, "dbf");
	hDBF = DBFOpen(szDBF, "rb");
	if (!hDBF)
	{
		SHPClose(hSHP);
		return in_flag;
	}

	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);

	SHPObject *psElem;
	double *padfX, *padfY, *padfZ;

	int nField;
	nField = DBFGetFieldCount(hDBF);
	polylines.resize(nEntities);

	for (int i = 0; i < nEntities; i++)
	{
		std::vector<Point3d>().swap(polylines[i]);

		psElem = SHPReadObject(hSHP, i);

		padfX = new double[psElem->nVertices];
		padfY = new double[psElem->nVertices];
		padfZ = new double[psElem->nVertices];
		for (int j = 0; j < psElem->nVertices; j++)
		{
			padfX[j] = psElem->padfX[j];
			padfY[j] = psElem->padfY[j];
			padfZ[j] = psElem->padfZ[j];

			Point3d point(padfX[j], padfY[j], padfZ[j]);
			polylines[i].push_back(point);
		}

		for (int j = 0; j < nField; j++)
		{
			DBFFieldType eType;
			int nWidth, nDecimals;
			char szTitle[20];

			eType = DBFGetFieldInfo(hDBF, j, szTitle, &nWidth, &nDecimals);
			switch (eType)
			{
				case FTString:
				{
					std::string value = DBFReadStringAttribute(hDBF, i, j);
					break;
				}
			
				case FTInteger:
				{
					int value = DBFReadIntegerAttribute(hDBF, i, j);
					break;
				}
				case FTDouble:
				{
					double value = DBFReadDoubleAttribute(hDBF, i, j);
					break;
				}
				default:
					break;
			}
		}

		delete[] padfX;
		delete[] padfY;
		delete[] padfZ;
		SHPDestroyObject(psElem);
	}

	// 关闭文件
	SHPClose(hSHP);
	DBFClose(hDBF);
}


int main()
{
	std::vector<std::vector<Point3d>> plys(2);
	// initialize
	for (int i = 0; i < plys.size(); ++i)
	{
		plys[i].resize(5);
		for (size_t j = 0; j < plys[i].size(); ++j)
		{
			plys[i][j].x= (i + 1.0) * (j + 1.0);
			plys[i][j].y = (i + 1.0) * (j + 1.0);
			plys[i][j].z = (i + 1.0) * (j + 1.0);
		}
	}
	std::string shp_stem = "test_polylines.shp";
	//bool success_shp_o = writeShapeFile(shp_stem, plys);

	std::vector<std::vector<Point3d>> plys_read;
	bool success_shp_i = readPolylines(shp_stem, plys_read);
	std::cout << "Polyline number is: " << plys_read.size() << std::endl;
	for (int i = 0; i < plys_read.size(); ++i)
	{
		std::cout << "Polyline: " << i << std::endl;
		for (int j = 0; j < plys_read[i].size(); ++j)
		{
			std::cout << "Point coordinate is: " << plys_read[i][j].x << ", " << plys_read[i][j].y << ", " << plys_read[i][j].z << std::endl;
		}
	}
    std::cout << "Hello World!\n";
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
