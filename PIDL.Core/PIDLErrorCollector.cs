using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PIDL
{
	public struct PIDLError
	{
		public int code;
		public string msg;
	}

	public interface IPIDLErrorCollector
	{
		void Add(int code, string msg);
		void Add(PIDLError err);
	}
}
