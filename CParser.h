#pragma once
#include <iostream>
#include <Windows.h>

class CParser
{
public:
	CParser() { m_pFileContentBuff = nullptr; }
	virtual ~CParser() { if(m_pFileContentBuff != nullptr) delete[] m_pFileContentBuff; }

	bool LoadFile(LPCWSTR fileName);
	bool FindCategory(LPCWSTR categoryName);
	bool GetValue_Int(LPCWSTR targetWord, int* pValue);
	bool GetValue_String(LPCWSTR targetWord, LPWSTR pBuffer);




private:
	bool SkipNoneCommand();
	bool GetNextWord(WCHAR** ppBuff, int* pLength);
	bool GetNextStringWord(WCHAR** ppBuff, int* pLength);

private:

	FILE*	m_fp;
	WCHAR*	m_pFileContentBuff;
	WCHAR* m_pFront;
	WCHAR* m_pCategoryStart;
};


// 사용법 :
// 1. LoadFile()을 통해 파일을 읽어들인다.
// 2. FindCategory()를 통해 원하는 카테고리를 선택한다.
// 3. 해당 카테고리에서 얻고 싶은 값의 이름을 GetValue_~()를 통해 얻는다.