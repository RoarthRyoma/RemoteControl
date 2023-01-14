#pragma once
class CEdoyunTool
{
public:
	static void Dump(BYTE* pData, size_t nSize)
	{
		std::string strOut;
		for (size_t i = 0; i < nSize; i++)
		{
			char buffer[8] = "";
			if (i > 0 && i % 16 == 0) strOut += "\n";
			snprintf(buffer, sizeof(buffer), "%02X ", pData[i] & 0xFF);
			strOut += buffer;
		}
		strOut += "\n";
		OutputDebugStringA(strOut.c_str());
	}
};

