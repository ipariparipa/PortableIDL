
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

    extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, const rapidjson::Value *& ret)
    {
        if (!r.HasMember(name))
            return false;
        ret = &r[name];
        return true;
    }

    extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, rapidjson::Value *&ret)
    {
        ret = (rapidjson::Value *)&v;
        return true;
    }

    extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, const rapidjson::Value *&ret)
    {
        ret = &v;
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

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, int & ret)
	{
		if (v.IsNull() || !v.IsNumber())
			return false;
        ret = v.IsInt() ? v.GetInt() : static_cast<int>(v.GetDouble());
        return true;
	}

    extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, unsigned int & ret)
    {
        if (v.IsNull() || !v.IsNumber())
            return false;

        if(v.IsUint())
            ret = v.GetUint();
        else if(v.IsInt() || v.GetDouble() < 0)
            return false;
        else
            ret = static_cast<unsigned int>(v.GetDouble());

        return true;
    }

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, long long & ret)
	{
		if (v.IsNull() || !v.IsNumber())
			return false;
        ret = v.IsInt64() ? v.GetInt64() : static_cast<long long>(v.GetDouble());
        return true;
	}

    extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, unsigned long long & ret)
    {
        if (v.IsNull() || !v.IsNumber())
            return false;

        if(v.IsUint64())
            ret = v.GetUint64();
        else if(v.IsInt64() || v.GetDouble() < 0)
            return false;
        else
            ret = static_cast<unsigned long long>(v.GetDouble());

        return true;
    }

	extern PIDL_CORE__FUNCTION  bool getValue(const rapidjson::Value & v, double & ret)
	{
		if (v.IsNull() || !v.IsNumber())
			return false;
		ret = v.GetDouble();
		return true;
	}

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

        long long year, month, day, hour, minute, second, nanosecond;
		if (!getValue(r, "year", year) ||
			!getValue(r, "month", month) ||
			!getValue(r, "day", day) ||
			!getValue(r, "hour", hour) ||
			!getValue(r, "minute", minute) ||
			!getValue(r, "second", second))
			return false;

        nanosecond = 0;
        if(!getValue(r, "nanosecond", nanosecond))
        {
            long long millisecond;
            if(getValue(r, "millisecond", millisecond))
                nanosecond = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(millisecond)).count();
        }

		memset(&t, 0, sizeof(tm));
        t.tm_year = static_cast<int>(year - 1900);
        t.tm_mon = static_cast<int>(month - 1);
        t.tm_mday = static_cast<int>(day - 1);
        t.tm_hour = static_cast<int>(hour);
        t.tm_min = static_cast<int>(minute);
        t.tm_sec = static_cast<int>(second);
        ret_millisecond = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds(nanosecond)).count());

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

        long long year, month, day, hour, minute, second, nanosecond;
		if (!getValue(r, "year", year) ||
			!getValue(r, "month", month) ||
			!getValue(r, "day", day) ||
			!getValue(r, "hour", hour) ||
			!getValue(r, "minute", minute) ||
			!getValue(r, "second", second))
			return false;

        nanosecond = 0;
        if(!getValue(r, "nanosecond", nanosecond))
        {
            long long millisecond;
            if(getValue(r, "millisecond", millisecond))
                nanosecond = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(millisecond)).count();
        }

        ret.year = static_cast<short>(year);
        ret.month = static_cast<short>(month);
        ret.day = static_cast<short>(day);
        ret.hour = static_cast<short>(hour);
        ret.minute = static_cast<short>(minute);
        ret.second = static_cast<short>(second);
        ret.nanosecond = static_cast<int>(nanosecond);

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

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, std::vector<char> & ret)
	{
		if (v.IsNull() || !v.IsString())
			return false;
		base64::decode(v.GetString(), ret);
		return true;
	}

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

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const std::string & str)
	{
		return setString(doc, str.c_str());
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, long long num)
	{
        (void)doc;
		rapidjson::Value v(rapidjson::kNumberType);
		v.SetInt64(num);
		return v;
	}

    extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, unsigned long long num)
    {
        (void)doc;
        rapidjson::Value v(rapidjson::kNumberType);
        v.SetUint64(num);
        return v;
    }

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, int num)
	{
        (void)doc;
        rapidjson::Value v(rapidjson::kNumberType);
		v.SetInt(num);
		return v;
	}

    extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, unsigned int num)
    {
        (void)doc;
        rapidjson::Value v(rapidjson::kNumberType);
        v.SetUint(num);
        return v;
    }

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, double num)
	{
        (void)doc;
        rapidjson::Value v(rapidjson::kNumberType);
		v.SetDouble(num);
		return v;
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value createNull(rapidjson::Document & doc)
	{
        (void)doc;
        return rapidjson::Value(rapidjson::kNullType);
	}
	extern PIDL_CORE__FUNCTION void addNull(rapidjson::Document & doc, rapidjson::Value & r, const char * name)
	{
		auto v = createNull(doc);
		addValue(doc, r, name, v);
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, bool b)
	{
        (void)doc;
        return rapidjson::Value(b ? rapidjson::kTrueType : rapidjson::kFalseType);
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const tm & t, short millisecond)
	{
		rapidjson::Value v(rapidjson::kObjectType);

        addValue(doc, v, "year", static_cast<long long>(t.tm_year + 1900));
        addValue(doc, v, "month", static_cast<long long>(t.tm_mon + 1));
        addValue(doc, v, "day", static_cast<long long>(t.tm_mday));
        addValue(doc, v, "hour", static_cast<long long>(t.tm_hour));
        addValue(doc, v, "minute", static_cast<long long>(t.tm_min));
        addValue(doc, v, "second", static_cast<long long>(t.tm_sec));
		if (millisecond > 0)
            addValue(doc, v, "millisecond", static_cast<long long>(millisecond));

		addValue(doc, v, "kind", "local");

		return v;
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const tm & t)
	{
		return createValue(doc, t, 0);
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const DateTime & dt)
	{
		rapidjson::Value v(rapidjson::kObjectType);

        addValue(doc, v, "year", static_cast<long long>(dt.year));
        addValue(doc, v, "month", static_cast<long long>(dt.month));
        addValue(doc, v, "day", static_cast<long long>(dt.day));
        addValue(doc, v, "hour", static_cast<long long>(dt.hour));
        addValue(doc, v, "minute", static_cast<long long>(dt.minute));
        addValue(doc, v, "second", static_cast<long long>(dt.second));
        if (dt.nanosecond > 0)
        {
            addValue(doc, v, "nanosecond", static_cast<long long>(dt.nanosecond));
            addValue(doc, v, "millisecond", static_cast<long long>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds(dt.nanosecond)).count()));
        }

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

