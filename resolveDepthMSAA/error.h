#include <iostream>
using namespace std;

namespace engine
{
	wstring errorString;
};

void Error(LPCWSTR txt)
{
	MessageBox(0,txt,L"Error!",0);
}

class ierror
{
public:
	ierror operator << (const wchar_t* c)
	{
		engine::errorString += c;

		if (engine::errorString[engine::errorString.size()-1]==L'\n')
		{
			Error(engine::errorString.c_str());
			engine::errorString = L"";
		}

		return *this;
	}

	ierror operator << (const char* c)
	{
		int len = strlen(c);
		wchar_t* buff = new wchar_t[len+1];
		mbstowcs(buff,c,len+1);

		this->operator<<(buff);

		delete[] buff;

		return *this;
	}

	ierror operator << (int i)
	{
		return this->operator<<(L"999");
	}
};