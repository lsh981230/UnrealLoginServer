#include <Windows.h>
#include <iostream>
#include "CParser.h"



bool CParser::LoadFile(LPCWSTR fileName)
{
	_wfopen_s(&m_fp, fileName, L"r");

	if (m_fp == nullptr)
		return false;

	// ���� ������ ������ ������
	fseek(m_fp, 0, SEEK_END);


	// ���� ������ �ִ� ������ ������ ���ϱ�
	int size = ftell(m_fp);


	// ���� ����
	rewind(m_fp);


	// ���� �Ҵ�
	m_pFileContentBuff = new WCHAR[size];


	// ���� �о���̱�
	fread(m_pFileContentBuff, size, 1, m_fp);

	// ���� �ݱ�
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


	if (*(WORD*)m_pFileContentBuff == 0xFEFF)		// BOM �ڵ尡 ���ִ� ���
	{
		m_pFront = &m_pFileContentBuff[1];
	}



	for (;;)
	{
		// ���ϴ� �ܾ ã�������� �˻�
		GetNextWord(&pWordStartPointer, &length);


		// ã�� �ܾ word ���ۿ� ����
		memset(word, 0, 128);
		memcpy_s(word, 128, pWordStartPointer, length);

		// ã�� �ܾ ���ϴ� �ܾ����� �˻�
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

				if (*m_pFront == 0x0a || *m_pFront == 0x0d)	// ���� �Է� ��
				{
					jump2NextLine = false;
					break;
				}

				m_pFront += 1;
			}
		}

		if (*m_pFront == 0x20 || *m_pFront == 0x09 || *m_pFront == 0x0a || *m_pFront == 0x0d)		// ���� / �� / ���๮�� �Է� ��
		{
			m_pFront += 1;
			continue;
		}

		else if (*m_pFront == 0x2f)							// ������ �Է� ��
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
		// ���ϴ� �ܾ ã�������� �˻�
		GetNextWord(&pWordStartPointer, &length);

		// ã�� �ܾ word ���ۿ� ����
		memset(word, 0, 128);
		memcpy_s(word, length, pWordStartPointer, length);

		// ã�� �ܾ ���ϴ� �ܾ����� �˻�
		if (wcscmp(targetWord, word) == 0)
		{
			// ������ �ڿ� �ִ� = �� ã��
			if (GetNextWord(&pWordStartPointer, &length))
			{
				memset(word, 0, 128);
				memcpy_s(word, length, pWordStartPointer, length);

				//  =�� ã�Ҵ��� �˻�
				if (wcscmp(word, L"=") == 0)
				{
					// = ������ ������ �κ� ���
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
		// ���ϴ� �ܾ ã�������� �˻�
		GetNextWord(&pWordStartPointer, &length);

		// ã�� �ܾ word ���ۿ� ����
		memset(word, 0, 128);
		memcpy_s(word, length, pWordStartPointer, length);

		// ã�� �ܾ ���ϴ� �ܾ����� �˻�
		if (wcscmp(targetWord, word) == 0)
		{
			// ������ �ڿ� �ִ� = �� ã��
			if (GetNextWord(&pWordStartPointer, &length))
			{
				memset(word, 0, 128);
				memcpy_s(word, length, pWordStartPointer, length);

				//  =�� ã�Ҵ��� �˻�
				if (wcscmp(word, L"=") == 0)
				{
					// = ������ ������ �κ� ���
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
