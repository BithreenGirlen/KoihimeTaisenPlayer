#ifndef STATIC_STRING_H_
#define STATIC_STRING_H_

#include <string_view>

/// @brief 動的割り当てを行わない文字列操作
template<size_t N>
class StaticString
{
public:
	const char* data() const { return m_data; }
	size_t size() const { return m_nWritten; }
	bool empty() const { return m_nWritten == 0; }
	const char front() const { return m_data[0]; }
	const char back() const { return m_data[m_nWritten]; }

	std::string_view string_view() const
	{
		return std::string_view(m_data, m_nWritten);
	}
	/// @brief 文字列連結
	StaticString& append(std::string_view s)
	{
		if (m_nWritten + s.size() > MaxSize)return *this;

		memcpy(m_data + m_nWritten, s.data(), s.size());
		m_nWritten += s.size();
		m_data[m_nWritten] = '\0';

		return *this;
	}
	/// @brief 文字列連結
	StaticString& append(const char* s, size_t length)
	{
		if (m_nWritten + length > MaxSize)return *this;

		memcpy(m_data + m_nWritten, s, length);
		m_nWritten += length;
		m_data[m_nWritten] = '\0';

		return *this;
	}
	/// @brief 文字連結
	void push_back(const char c)
	{
		if (m_nWritten + 1 > MaxSize)return;

		m_data[m_nWritten] = c;
		++m_nWritten;
		m_data[m_nWritten] = '\0';
	}
	/// @brief 文字挿入
	void insert(const char c, size_t nPos = 0)
	{
		if (m_nWritten + 1 > MaxSize)return;

		memmove(&m_data[nPos + 1], &m_data[nPos], m_nWritten - nPos);
		m_data[nPos] = c;
		++m_nWritten;
	}
	/// @brief 文字列挿入
	void insert(std::string_view s, size_t nPos = 0)
	{
		if (s.size() + nPos > MaxSize)return;

		memmove(&m_data[nPos + s.size()], &m_data[nPos], m_nWritten - nPos);
		memcpy(&m_data[nPos], s.data(), s.size());
		m_nWritten += s.size();
	}
	/// @brief 文字置換
	void replace(const char cOld, const char cNew)
	{
		for (size_t i = 0; i < m_nWritten; ++i)
		{
			char& cRef = m_data[i];
			if (cRef == cOld)
			{
				cRef = cNew;
			}
		}
	}
	/// @brief 文字列置換
	void replace(std::string_view strOld, std::string_view strNew)
	{
		if (strOld.empty())return;

		for (size_t nLast = 0; nLast < m_nWritten;)
		{
			std::string_view s = string_view();
			size_t nPos = s.find(strOld, nLast);
			if (nPos == std::string_view::npos)break;

			ptrdiff_t nDiff = static_cast<ptrdiff_t>(strNew.size() - strOld.size());
			if (m_nWritten + nDiff > MaxSize) break;

			char* pPos = m_data + nPos;
			memmove(pPos + strNew.size(), pPos + strOld.size(), m_nWritten - nPos - strOld.size() + 1);
			memcpy(pPos, strNew.data(), strNew.size());

			m_nWritten += nDiff;
			nLast = nPos + strNew.size();
		}
	}
	/// @brief 消去
	void clear()
	{
		memset(m_data, '\0', MaxSize);
		m_nWritten = 0;
	}
	/// @brief 縮め
	void shrink(size_t nLength)
	{
		if (nLength >= m_nWritten)return;

		memset(m_data + nLength, '\0', MaxSize - nLength);
		m_nWritten = nLength;
	}
private:
	char m_data[N]{};
	size_t m_nWritten = 0;
	static constexpr size_t MaxSize = sizeof(m_data) - 1;
};

/// @brief 動的割り当てを行わない文字列操作
template<size_t N>
class StaticWString
{
public:
	const wchar_t* data() const { return m_data; }
	size_t size() const { return m_nWritten; }
	bool empty() const { return m_nWritten == 0; }
	const wchar_t front() const { return m_data[0]; }
	const wchar_t back() const { return m_data[m_nWritten]; }

	std::wstring_view string_view() const
	{
		return std::wstring_view(m_data, m_nWritten);
	}
	/// @brief 文字列連結
	StaticWString& append(std::wstring_view s)
	{
		if (m_nWritten + s.size() > MaxSize)return *this;

		wmemcpy(m_data + m_nWritten, s.data(), s.size());
		m_nWritten += s.size();
		m_data[m_nWritten] = L'\0';

		return *this;
	}
	/// @brief 文字列連結
	StaticWString& append(const wchar_t* s, size_t length)
	{
		if (m_nWritten + length > MaxSize)return *this;

		wmemcpy(m_data + m_nWritten, s, length);
		m_nWritten += length;
		m_data[m_nWritten] = L'\0';

		return *this;
	}
	/// @brief 文字連結
	void push_back(const wchar_t c)
	{
		if (m_nWritten + 1 > MaxSize)return;

		m_data[m_nWritten] = c;
		++m_nWritten;
		m_data[m_nWritten] = L'\0';
	}
	/// @brief 文字挿入
	void insert(const wchar_t c, size_t nPos = 0)
	{
		if (m_nWritten + 1 > MaxSize)return;

		wmemmove(&m_data[nPos + 1], &m_data[nPos], m_nWritten - nPos);
		m_data[nPos] = c;
		++m_nWritten;
	}
	/// @brief 文字列挿入
	void insert(std::string_view s, size_t nPos = 0)
	{
		if (s.size() + nPos > MaxSize)return;

		wmemmove(&m_data[nPos + s.size()], &m_data[nPos], m_nWritten - nPos);
		wmemcpy(&m_data[nPos], s.data(), s.size());
		m_nWritten += s.size();
	}
	/// @brief 文字置換
	void replace(const wchar_t cOld, const wchar_t cNew)
	{
		for (size_t i = 0; i < m_nWritten; ++i)
		{
			wchar_t& cRef = m_data[i];
			if (cRef == cOld)
			{
				cRef = cNew;
			}
		}
	}
	/// @brief 文字列置換
	void replace(std::wstring_view strOld, std::wstring_view strNew)
	{
		if (strOld.empty())return;

		for (size_t nLast = 0; nLast < m_nWritten;)
		{
			std::wstring_view s = string_view();
			size_t nPos = s.find(strOld, nLast);
			if (nPos == std::wstring_view::npos)break;

			ptrdiff_t nDiff = static_cast<ptrdiff_t>(strNew.size() - strOld.size());
			if (m_nWritten + nDiff > MaxSize) break;

			wchar_t* pPos = m_data + nPos;
			wmemmove(pPos + strNew.size(), pPos + strOld.size(), m_nWritten - nPos - strOld.size() + 1);
			wmemcpy(pPos, strNew.data(), strNew.size());

			m_nWritten += nDiff;
			nLast = nPos + strNew.size();
		}
	}
	/// @brief 消去
	void clear()
	{
		wmemset(m_data, L'\0', MaxSize);
		m_nWritten = 0;
	}
	/// @brief 縮め
	void shrink(size_t nLength)
	{
		if (nLength >= m_nWritten)return;

		wmemset(m_data + nLength, L'\0', MaxSize - nLength);
		m_nWritten = nLength;
	}
private:
	wchar_t m_data[N]{};
	size_t m_nWritten = 0;
	static constexpr size_t MaxSize = sizeof(m_data) / sizeof(wchar_t) - 1;
};

using StaticString512 = StaticString<512>;
using StaticWString512 = StaticWString<512>;

#endif // !STATIC_STRING_H_
