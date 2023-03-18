#include <Windows.h>
#include <iostream>
#include "CParser.h"



bool CParser::LoadFile(LPCWSTR fileName)
{
	_wfopen_s(&m_fp, fileName, L"r");

	if (m_fp == nullptr)
		return false;

	// 파일 포인터 끝으로 보내기
	fseek(m_fp, 0, SEEK_END);


	// 파일 포인터 있는 곳까지 사이즈 구하기
	int size = ftell(m_fp);


	// 버퍼 비우기
	rewind(m_fp);


	// 버퍼 할당
	m_pFileContentBuff = new WCHAR[size];


	// 내용 읽어들이기
	fread(m_pFileContentBuff, size, 1, m_fp);

	// 파일 닫기
	fclose(m_fp);

	return true;
}


bool CParser::FindCategory(LPCWSTR categoryName)
{
	m_pFront = m_pFileContentBuff;
	m_pCategoryStart = nullptr;

	WCHAR* pWordStartPointer = nullptr;
	WCHAR  word[128];
	int	  length;


	if (*(WORD*)m_pFileContentBuff == 0xFEFF)		// BOM 코드가 들어가있는 경우
	{
		m_pFront = &m_pFileContentBuff[1];
	}



	for (;;)
	{
		// 원하는 단어를 찾을때까지 검사
		GetNextWord(&pWordStartPointer, &length);


		// 찾은 단어를 word 버퍼에 저장
		memset(word, 0, 128);
		memcpy_s(word, 128, pWordStartPointer, length);

		// 찾은 단어가 원하는 단어인지 검사
		if (wcscmp(categoryName, word) == 0)
		{
			GetNextWord(&pWordStartPointer, &length);

			memset(word, 0, 128);
			memcpy_s(word, length, pWordStartPointer, length);

			if (wcscmp(L"{", word) == 0)
				break;
		}
	}

	m_pCategoryStart = pWordStartPointer;

	return true;
}

bool CParser::SkipNoneCommand()
{
	bool getSlash = false;
	bool jump2NextLine = false;

	for (;;)
	{
		// If EOF
		if (*m_pFront == '}' && m_pCategoryStart)
			return false;

		if (jump2NextLine)
		{
			for (;;)
			{
				// If EOF
				if (*m_pFront == '}' && m_pCategoryStart)
					return false;

				if (*m_pFront == 0x0a || *m_pFront == 0x0d)	// 개행 입력 시
				{
					jump2NextLine = false;
					break;
				}

				m_pFront += 1;
			}
		}

		if (*m_pFront == 0x20 || *m_pFront == 0x09 || *m_pFront == 0x0a || *m_pFront == 0x0d)		// 공백 / 탭 / 개행문자 입력 시
		{
			m_pFront += 1;
			continue;
		}

		else if (*m_pFront == 0x2f)							// 슬래시 입력 시
		{
			if (getSlash)
				jump2NextLine = true;
			else
				getSlash = true;

			m_pFront += 1;
		}
		else
			break;
	}

	return true;
}


bool CParser::GetNextWord(WCHAR** ppWordStart, int* pLength)
{
	*pLength = 0;

	if (SkipNoneCommand() == false)
		return false;

	*ppWordStart = m_pFront;

	for (;;)
	{
		// If EOF
		if (*m_pFront == '}' && m_pCategoryStart)
			return false;


		//  If Find Target
		if (*m_pFront == ',' || *m_pFront == 0x20 || *m_pFront == 0x08 || *m_pFront == 0x09 || *m_pFront == 0x0a || *m_pFront == 0x0d)
		{
			break;
		}

		m_pFront += 1;
		*pLength += 2;
	}

	return true;
}



bool CParser::GetNextStringWord(WCHAR** ppWordStart, int* pLength)
{
	bool findWord = false;
	*pLength = -2;

	if (SkipNoneCommand() == false)
		return false;

	*ppWordStart = m_pFront + 1;

	for (;;)
	{
		// If EOF
		if (*m_pFront == '}')
			return false;



		//  If Find Target
		if (*m_pFront == '"')
		{
			if (findWord)
				break;
			else
				findWord = true;
		}


		if (findWord)
		{
			*pLength += 2;
		}

		m_pFront += 1;
	}

	return true;
}


bool CParser::GetValue_Int(LPCWSTR targetWord, int* pValue)
{
	WCHAR* pWordStartPointer = nullptr;
	WCHAR  word[128];
	int	  length;

	m_pFront = m_pCategoryStart;

	for (;;)
	{
		// 원하는 단어를 찾을때까지 검사
		GetNextWord(&pWordStartPointer, &length);

		// 찾은 단어를 word 버퍼에 저장
		memset(word, 0, 128);
		memcpy_s(word, length, pWordStartPointer, length);

		// 찾은 단어가 원하는 단어인지 검사
		if (wcscmp(targetWord, word) == 0)
		{
			// 맞으면 뒤에 있는 = 을 찾음
			if (GetNextWord(&pWordStartPointer, &length))
			{
				memset(word, 0, 128);
				memcpy_s(word, length, pWordStartPointer, length);

				//  =을 찾았는지 검사
				if (wcscmp(word, L"=") == 0)
				{
					// = 다음의 데이터 부분 얻기
					if (GetNextWord(&pWordStartPointer, &length))
					{
						memset(word, 0, 128);
						memcpy(word, pWordStartPointer, length);
						*pValue = _wtoi(word);
						m_pCategoryStart = m_pFront;
						return true;
					}
					return false;
				}
			}
			return false;
		}
	}

	return false;
}






bool CParser::GetValue_String(LPCWSTR targetWord, LPWSTR pBuffer)
{
	WCHAR* pWordStartPointer;
	WCHAR  word[128];
	int	  length;

	m_pFront = m_pCategoryStart;

	for (;;)
	{
		// 원하는 단어를 찾을때까지 검사
		GetNextWord(&pWordStartPointer, &length);

		// 찾은 단어를 word 버퍼에 저장
		memset(word, 0, 128);
		memcpy_s(word, length, pWordStartPointer, length);

		// 찾은 단어가 원하는 단어인지 검사
		if (wcscmp(targetWord, word) == 0)
		{
			// 맞으면 뒤에 있는 = 을 찾음
			if (GetNextWord(&pWordStartPointer, &length))
			{
				memset(word, 0, 128);
				memcpy_s(word, length, pWordStartPointer, length);

				//  =을 찾았는지 검사
				if (wcscmp(word, L"=") == 0)
				{
					// = 다음의 데이터 부분 얻기
					if (GetNextStringWord(&pWordStartPointer, &length))
					{
						memcpy_s(pBuffer, length, pWordStartPointer, length);
						m_pCategoryStart = m_pFront;
						return true;
					}
					return false;
				}
			}
			return false;
		}
	}

	return false;
}
