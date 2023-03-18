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


// ���� :
// 1. LoadFile()�� ���� ������ �о���δ�.
// 2. FindCategory()�� ���� ���ϴ� ī�װ��� �����Ѵ�.
// 3. �ش� ī�װ����� ��� ���� ���� �̸��� GetValue_~()�� ���� ��´�.