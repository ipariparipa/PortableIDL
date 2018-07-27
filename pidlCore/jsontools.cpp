
#include "include/pidlCore/jsontools.h"

namespace PIDL { namespace JSONTools {

	namespace base64 {
		static const std::string base64_chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		typedef char byte;

		static inline bool is_base64(byte c) {
			return (isalnum(c) || (c == '+') || (c == '/'));
		}

		std::string & encode(const std::vector<byte> & bin, std::string & ret) {

			byte const* bytes_to_encode = bin.data();
			size_t in_len = bin.size();

			int i = 0;
			int j = 0;
			byte char_array_3[3];
			byte char_array_4[4];


			while (in_len--) {
				char_array_3[i++] = *(bytes_to_encode++);
				if (i == 3) {
					char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
					char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
					char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
					char_array_4[3] = char_array_3[2] & 0x3f;

					for (i = 0; (i < 4); i++)
						ret += base64_chars[char_array_4[i]];
					i = 0;
				}
			}

			if (i)
			{
				for (j = i; j < 3; j++)
					char_array_3[j] = '\0';

				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (j = 0; (j < i + 1); j++)
					ret += base64_chars[char_array_4[j]];

				while ((i++ < 3))
					ret += '=';

			}

			return ret;

		}

		std::vector<char> & decode(std::string const& encoded_string, std::vector<byte> & ret)
		{
			size_t in_len = encoded_string.size();
			size_t i = 0;
			size_t j = 0;
			int in_ = 0;
			byte char_array_4[4], char_array_3[3];

			while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
				char_array_4[i++] = encoded_string[in_]; in_++;
				if (i == 4) {
					for (i = 0; i < 4; i++)
						char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

					char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
					char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
					char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

					for (i = 0; (i < 3); i++)
						ret.push_back(char_array_3[i]);
					i = 0;
				}
			}

			if (i) {
				for (j = i; j < 4; j++)
					char_array_4[j] = 0;

				for (j = 0; j < 4; j++)
					char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
			}

			return ret;
		}
	}

	extern PIDL_CORE__FUNCTION std::string getErrorText(rapidjson::ParseErrorCode code)
	{
		switch (code)
		{
		case rapidjson::kParseErrorNone: return "No error.";

		case rapidjson::kParseErrorDocumentEmpty: return"The document is empty";
		case rapidjson::kParseErrorDocumentRootNotSingular: return "The document root must not follow by other values.";

		case rapidjson::kParseErrorValueInvalid: return "Invalid value.";

		case rapidjson::kParseErrorObjectMissName: return "Missing a name for object member.";
		case rapidjson::kParseErrorObjectMissColon: return "Missing a colon after a name of object member.";
		case rapidjson::kParseErrorObjectMissCommaOrCurlyBracket: return "Missing a comma or '}' after an object member.";

		case rapidjson::kParseErrorArrayMissCommaOrSquareBracket: return "Missing a comma or ']' after an array element.";

		case rapidjson::kParseErrorStringUnicodeEscapeInvalidHex: return "Incorrect hex digit after \\u escape in string.";
		case rapidjson::kParseErrorStringUnicodeSurrogateInvalid: return "The surrogate pair in string is invalid.";
		case rapidjson::kParseErrorStringEscapeInvalid: return "Invalid escape character in string.";
		case rapidjson::kParseErrorStringMissQuotationMark: return "Missing a closing quotation mark in string.";
		case rapidjson::kParseErrorStringInvalidEncoding: return "Invalid encoding in string.";

		case rapidjson::kParseErrorNumberTooBig: return "Number too big to be stored in double.";
		case rapidjson::kParseErrorNumberMissFraction: return "Miss fraction part in number.";
		case rapidjson::kParseErrorNumberMissExponent: return "Miss exponent in number.";

		case rapidjson::kParseErrorTermination: return "Parsing was terminated.";
		case rapidjson::kParseErrorUnspecificSyntaxError: return "Unspecific syntax error.";

		}

		return std::string() + "unknown error code: '" + std::to_string(code) + "'";
	}


	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, rapidjson::Value *& ret)
	{
		if (!r.HasMember(name))
			return false;
		ret = (rapidjson::Value *)&r[name];
		return true;
	}

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, std::string & ret)
	{
		rapidjson::Value * v;
		if (!getValue(r, name, v))
			return false;

		return getValue(*v, ret);
	}

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, std::string & ret)
	{
		if (v.IsNull() || !v.IsString())
			return false;
		ret = v.GetString();
		return true;
	}

	//extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, long long & ret)
	//{
	//	rapidjson::Value * v;
	//	if (!getValue(r, name, v))
	//		return false;

	//	return getValue(*v, ret);
	//}

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, long long & ret)
	{
		if (v.IsNull() || !v.IsNumber())
			return false;
		ret = v.GetInt64();
		return true;
	}


	//extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, int & ret)
	//{
	//	if (v.IsNull() || !v.IsNumber())
	//		return false;
	//	ret = v.GetInt();
	//	return true;
	//}

	//extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, int & ret)
	//{
	//	rapidjson::Value * v;
	//	if (!getValue(r, name, v))
	//		return false;

	//	return getValue(*v, ret);
	//}

	//extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, double & ret)
	//{
	//	rapidjson::Value * v;
	//	if (!getValue(r, name, v))
	//		return false;

	//	return getValue(*v, ret);
	//}

	extern PIDL_CORE__FUNCTION  bool getValue(const rapidjson::Value & v, double & ret)
	{
		if (v.IsNull() || !v.IsNumber())
			return false;
		ret = v.GetDouble();
		return true;
	}

	//extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, bool & ret)
	//{
	//	rapidjson::Value * v;
	//	if (!getValue(r, name, v))
	//		return false;
	//	return getValue(*v, ret);
	//}

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, bool & ret)
	{
		if (v.IsNull() || !v.IsBool())
			return false;
		ret = v.GetBool();
		return true;
	}

	extern bool PIDL_CORE__FUNCTION getValue(const rapidjson::Value & r, tm & t, int & ret_millisecond)
	{
		if (r.IsNull() || !r.IsObject())
			return false;

		long long year, month, day, hour, minute, second, millisecond;
		if (!getValue(r, "year", year) ||
			!getValue(r, "month", month) ||
			!getValue(r, "day", day) ||
			!getValue(r, "hour", hour) ||
			!getValue(r, "minute", minute) ||
			!getValue(r, "second", second))
			return false;

		millisecond = 0;
		getValue(r, "millisecond", millisecond);

		memset(&t, 0, sizeof(tm));
		t.tm_year = year - 1900;
		t.tm_mon = month - 1;
		t.tm_mday = day - 1;
		t.tm_hour = hour;
		t.tm_min = minute;
		t.tm_sec = second;
		ret_millisecond = (int)millisecond;

		return true;
	}

	extern bool PIDL_CORE__FUNCTION getValue(const rapidjson::Value & r, tm & t)
	{
		int millisecond;
		return getValue(r, t, millisecond);
	}

	extern bool PIDL_CORE__FUNCTION getValue(const rapidjson::Value & r, DateTime & ret)
	{
		if (r.IsNull() || !r.IsObject())
			return false;

		long long year, month, day, hour, minute, second, millisecond;
		if (!getValue(r, "year", year) ||
			!getValue(r, "month", month) ||
			!getValue(r, "day", day) ||
			!getValue(r, "hour", hour) ||
			!getValue(r, "minute", minute) ||
			!getValue(r, "second", second))
			return false;

		millisecond = 0;
		getValue(r, "millisecond", millisecond);

		ret.year = (short)year;
		ret.month = (short)month;
		ret.day = (short)day;
		ret.hour = (short)hour;
		ret.minute = (short)minute;
		ret.second = (short)second;
		ret.millisecond = (short)millisecond;

		std::string kind_str;
		if (getValue(r, "kind", kind_str))
		{
			if (kind_str == "local")
				ret.kind = DateTime::Local;
			else if (kind_str == "utc")
				ret.kind = DateTime::UTC;
			else if (kind_str == "none")
				ret.kind = DateTime::None;
		}
		else
			ret.kind = DateTime::None;

		return true;
	}

	//extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, tm & ret)
	//{
	//	rapidjson::Value * v;
	//	if (!getValue(r, name, v))
	//		return false;

	//	return getValue(*v, ret);
	//}

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, std::vector<char> & ret)
	{
		if (v.IsNull() || !v.IsString())
			return false;
		base64::decode(v.GetString(), ret);
		return true;
	}

	//extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, std::vector<char> & ret)
	//{
	//	rapidjson::Value * v;
	//	if (!getValue(r, name, v))
	//		return false;

	//	return getValue(*v, ret);
	//}



	extern PIDL_CORE__FUNCTION rapidjson::Value setString(rapidjson::Document & doc, const char * str)
	{
		rapidjson::Value v(rapidjson::kStringType);
		v.SetString(str, doc.GetAllocator());
		return v;
	}

	extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, rapidjson::Value & v)
	{
		r.AddMember(setString(doc, name), v, doc.GetAllocator());
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const char * str)
	{
		return setString(doc, str);
	}
	//extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const char * str)
	//{
	//	auto v = createValue(doc, str);
	//	addValue(doc, r, name, v);
	//}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const std::string & str)
	{
		return setString(doc, str.c_str());
	}
	//extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::string & str)
	//{
	//	auto v = createValue(doc, str);
	//	addValue(doc, r, name, v);
	//}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, long long num)
	{
		rapidjson::Value v(rapidjson::kNumberType);
		v.SetInt64(num);
		return v;
	}
	//extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, long long num)
	//{
	//	auto v = createValue(doc, num);
	//	addValue(doc, r, name, v);
	//}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, int num)
	{
		rapidjson::Value v(rapidjson::kNumberType);
		v.SetInt(num);
		return v;
	}
	//extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, int num)
	//{
	//	auto v = createValue(doc, num);
	//	addValue(doc, r, name, v);
	//}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, double num)
	{
		rapidjson::Value v(rapidjson::kNumberType);
		v.SetDouble(num);
		return v;
	}
	//extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, double num)
	//{
	//	auto v = createValue(doc, num);
	//	addValue(doc, r, name, v);
	//}

	extern PIDL_CORE__FUNCTION rapidjson::Value createNull(rapidjson::Document & doc)
	{
		return rapidjson::Value(rapidjson::kNullType);
	}
	extern PIDL_CORE__FUNCTION void addNull(rapidjson::Document & doc, rapidjson::Value & r, const char * name)
	{
		auto v = createNull(doc);
		addValue(doc, r, name, v);
	}

	//extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, bool b)
	//{
	//	auto v = createValue(doc, b);
	//	addValue(doc, r, name, v);
	//}
	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, bool b)
	{
		return rapidjson::Value(b ? rapidjson::kTrueType : rapidjson::kFalseType);
	}

	//extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const tm & t)
	//{
	//	auto v = createValue(doc, t);
	//	addValue(doc, r, name, v);
	//}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const tm & t, int millisecond)
	{
		rapidjson::Value v(rapidjson::kObjectType);

		addValue(doc, v, "year", t.tm_year + 1900);
		addValue(doc, v, "month", t.tm_mon + 1);
		addValue(doc, v, "day", t.tm_mday);
		addValue(doc, v, "hour", t.tm_hour);
		addValue(doc, v, "minute", t.tm_min);
		addValue(doc, v, "second", t.tm_sec);
		if (millisecond > 0)
			addValue(doc, v, "millisecond", millisecond);

		addValue(doc, v, "kind", "local");

		return v;
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const tm & t)
	{
		return createValue(doc, t, 0);
	}

	//extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const DateTime & t)
	//{
	//	auto v = createValue(doc, t);
	//	addValue(doc, r, name, v);
	//}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const DateTime & dt)
	{
		rapidjson::Value v(rapidjson::kObjectType);

		addValue(doc, v, "year", dt.year);
		addValue(doc, v, "month", dt.month);
		addValue(doc, v, "day", dt.day);
		addValue(doc, v, "hour", dt.hour);
		addValue(doc, v, "minute", dt.minute);
		addValue(doc, v, "second", dt.second);
		if (dt.millisecond > 0)
			addValue(doc, v, "millisecond", dt.millisecond);

		switch (dt.kind)
		{
		case DateTime::None:
			break;
		case DateTime::Local:
			addValue(doc, v, "kind", "local");
			break;
		case DateTime::UTC:
			addValue(doc, v, "kind", "utc");
			break;
		}

		return v;
	}


	extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<char> & b)
	{
		auto v = createValue(doc, b);
		addValue(doc, r, name, v);
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const std::vector<char> & b)
	{
		std::string tmp;
		return setString(doc, base64::encode(b, tmp).c_str());
	}

}}

