/*
    This file is part of PIDL.Core.

    PIDL.Core is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIDL.Core is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with PIDL.Core.  If not, see <http://www.gnu.org/licenses/>
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Xml.Linq;

namespace PIDL
{
	public static class JSONTools
	{
		public enum Type
		{
			None, Object, Array, String, Number, Bool, Null
		}

		public static Type getType(XElement v)
		{
			XAttribute a;
			if ((a = v.Attribute("type")) == null)
				return Type.None;

			if (a.Value == "string")
				return Type.String;
			if (a.Value == "number")
				return Type.Number;
			if (a.Value == "boolean")
				return Type.Bool;
			if (a.Value == "array")
				return Type.Array;
			if (a.Value == "object")
				return Type.Object;
			if (a.Value == "null")
				return Type.Null;

			return Type.None;
		}

		public static bool checkType(XElement v, Type t)
		{
			return getType(v) == t;
		}

		public static bool getValue(XElement r, string name, out XElement ret)
		{
			return (ret = r.Element(name)) != null;
		}

		public static bool getValue(XElement r, string name, Type t, out XElement ret)
		{
			if ((ret = r.Element(name)) == null)
				return false;
			return checkType(ret, t);
		}

		public static bool getValue<T>(XElement r, string name, out T ret)
		{
			XElement v;
			if (!getValue(r, name, out v))
			{
				ret = default(T);
				return false;
			}
			bool isOk;
			dynamic _ret = getValueX<T>(v, out isOk);
			if (!isOk)
			{
				ret = default(T);
				return false;
			}
			ret = _ret;
			return true;
		}

		private static T getValueX<T>(XElement v, out bool isOk)
		{
			Type t;
			if ((t = getType(v)) == Type.None)
			{
				isOk = true;
				return default(T);
			}

			if (typeof(T) == typeof(long))
			{
				if (t == Type.Number)
				{
					var ret = Convert.ToInt64(v.Value, new System.Globalization.NumberFormatInfo());
					isOk = true;
					return (T)(object)ret;
				}
			}
			else if (typeof(T) == typeof(double))
			{
				if (t == Type.Number)
				{
					var ret = Convert.ToDouble(v.Value, new System.Globalization.NumberFormatInfo());
					isOk = true;
					return (T)(object)ret;
				}
			}
			else if (typeof(T) == typeof(string))
			{
				if (t == Type.String)
				{
					var ret = v.Value;
					isOk = true;
					return (T)(object)ret;
				}
				else if (t == Type.Null)
				{
					isOk = true;
					return default(T);
				}
			}
			else if (typeof(T) == typeof(bool))
			{
				if (t == Type.Bool)
				{
					var ret = v.Value == "true";
					isOk = true;
					return (T)(object)ret;
				}
			}
			else if (typeof(T) == typeof(DateTime))
			{
				if (t == Type.Object)
				{
					long year, month, day, hour, minute, second, millisecond;

					if ((isOk = getValue(v, "year", out year)
							  & getValue(v, "month", out month)
							  & getValue(v, "day", out day)
							  & getValue(v, "hour", out hour)
							  & getValue(v, "minute", out minute)
							  & getValue(v, "second", out second)))
					{
						getValue(v, "millisecond", out millisecond);

						DateTimeKind kind = DateTimeKind.Unspecified;
						string kind_str;
						if(getValue(v, "kind", out kind_str))
						{
							if (kind_str == "local")
								kind = DateTimeKind.Local;
							else if (kind_str == "utc")
								kind = DateTimeKind.Utc;
							else if (kind_str == "none")
								kind = DateTimeKind.Unspecified;
							else
							{
								isOk = false;
								return default(T);
							}
						}

						var ret = new DateTime((int)year, (int)month, (int)day, (int)hour, (int)minute, (int)second, (int)millisecond, kind);
						return (T)(object)ret;
					}
				}
			}

			isOk = false;
			return default(T);
		}

		public static bool getValue<T>(XElement v, out T ret)
		{
			bool isOk;
			ret = getValueX<T>(v, out isOk);
			return isOk;
		}

		public static bool getValue<T>(XElement r, string name, out Nullable<T> ret)
			where T : struct, IComparable
		{
			XElement v;
			PIDL.JSONTools.Type t;
			if (!PIDL.JSONTools.getValue(r, name, out v) || (t = PIDL.JSONTools.getType(v)) == PIDL.JSONTools.Type.None)
			{
				ret = null;
				return false;
			}

			if (t == PIDL.JSONTools.Type.Null)
			{
				ret = null;
				return true;
			}

			return getValue(v, out ret);
		}

		public static bool getValue<T>(XElement r, string name, out T[] ret)
		{
			XElement v;
			PIDL.JSONTools.Type t;
			if (!PIDL.JSONTools.getValue(r, name, out v) || (t = PIDL.JSONTools.getType(v)) == Type.None)
			{
				ret = null;
				return false;
			}

			if (t == PIDL.JSONTools.Type.Null)
			{
				ret = null;
				return true;
			}

			if (typeof(T) == typeof(byte))
			{
				string tmp;
				if (!getValue(v, out tmp))
				{
					ret = null;
					return false;
				}
				ret = (T[])(object)Convert.FromBase64String(tmp);
				return true;
			}

			if (t != PIDL.JSONTools.Type.Array)
			{
				ret = null;
				return false;
			}

			var elems = v.Elements("item");
			int i = 0;
			foreach (var e in elems)
				++i;

			ret = new T[i];
			i = 0;
			foreach (var e in elems)
			{
				if (!getValue(e, out ret[i]))
					return false;
			}

			return true;
		}

		public static bool getValue(XElement v, out long ret)
		{
			bool isOk;
			ret = getValueX<long>(v, out isOk);
			return isOk;
		}

		public static bool getValue(XElement v, out double ret)
		{
			bool isOk;
			ret = getValueX<double>(v, out isOk);
			return isOk;
		}

		public static bool getValue(XElement v, out string ret)
		{
			bool isOk;
			ret = getValueX<string>(v, out isOk);
			return isOk;
		}

		public static bool getValue(XElement v, out bool ret)
		{
			bool isOk;
			ret = getValueX<bool>(v, out isOk);
			return isOk;
		}

		public static XElement addValue(XElement r, string name, Type t)
		{
			XElement ret;
			r.Add(ret = createValue(name, t));

			return ret;
		}

		public static XElement createValue(string name, Type t)
		{
			var v = new XElement(name);
			switch (t)
			{
				case Type.None:
				case Type.Null:
					v.SetAttributeValue("type", "null");
					break;
				case Type.Number:
					v.SetAttributeValue("type", "number");
					break;
				case Type.Object:
					v.SetAttributeValue("type", "object");
					break;
				case Type.String:
					v.SetAttributeValue("type", "string");
					break;
				case Type.Bool:
					v.SetAttributeValue("type", "boolean");
					break;
				case Type.Array:
					v.SetAttributeValue("type", "array");
					break;
			}
			return v;
		}

		public static void addValue<T>(XElement r, string name, T val)
		{
			if (val == null)
			{
				addValue(r, name, Type.Null);
				return;
			}

			XElement v;

			if (typeof(T) == typeof(long))
				(v = addValue(r, name, Type.Number)).Add(Convert.ToString(val, new System.Globalization.NumberFormatInfo()));
			else if (typeof(T) == typeof(double))
				(v = addValue(r, name, Type.Number)).Add(Convert.ToString(val, new System.Globalization.NumberFormatInfo()));
			else if (typeof(T) == typeof(string))
				(v = addValue(r, name, Type.String)).Add(val);
			else if (typeof(T) == typeof(bool))
				(v = addValue(r, name, Type.Bool)).Add(val.ToString().ToLower());
			else if (typeof(T) == typeof(DateTime))
			{
				var dt = (DateTime)(Object)val;
				v = addValue(r, name, Type.Object);
				addValue(v, "year", dt.Year);
				addValue(v, "month", dt.Month);
				addValue(v, "day", dt.Day);
				addValue(v, "hour", dt.Hour);
				addValue(v, "minute", dt.Minute);
				addValue(v, "second", dt.Second);
				if (dt.Millisecond > 0)
					addValue(v, "millisecond", dt.Millisecond);
				switch(dt.Kind)
				{
					case DateTimeKind.Local:
						addValue(v, "kind", "local");
						break;
					case DateTimeKind.Utc:
						addValue(v, "kind", "utc");
						break;
					case DateTimeKind.Unspecified:
						break;
				}
			}
			else
				throw new ArgumentException("invalid template argument");
		}

		public static void addValue<T>(XElement r, string name, Nullable<T> val)
			where T : struct, IComparable
		{
			if (val == null)
				addValue(r, name, Type.Null);
			else
				addValue<T>(r, name, val.Value);
		}

		public static void addValue<T>(XElement r, string name, T[] val)
		{
			if (val == null)
			{
				addValue(r, name, PIDL.JSONTools.Type.Null);
				return;
			}

			if (typeof(T) == typeof(byte))
			{
				addValue(r, name, Convert.ToBase64String((byte[])(object)val));
				return;
			}

			var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Array);
			foreach (var it in val)
				addValue(v, "item", it);
		}

	}
}
