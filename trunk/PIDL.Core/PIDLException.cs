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

namespace PIDL
{
	public class PIDLException : System.Exception
	{
		public PIDLException(PIDLError err, System.Exception inner = null)
			: base(null, inner)
		{
			var errs = new PIDLError[1];
			errs[0] = err;
			Errors = errs;
		}

		public PIDLException(IEnumerable<PIDLError> errs, System.Exception inner = null)
			: base(null, inner)
		{
			Errors = errs;
		}

		public IEnumerable<PIDLError> Errors { get; private set; }

		public override string Message
		{
			get
			{
				var strl = new List<string>();
				foreach (var e in Errors)
					strl.Add(string.Format("[{0}] {1}", e.code, e.msg));
				return string.Join("\n", strl);
			}
		}

		public void Get(IPIDLErrorCollector ec)
		{
			foreach (var e in Errors)
				ec.Add(e);
		}
	}

	public class PIDLExceptionErrorCollector : IPIDLErrorCollector
	{
		public PIDLExceptionErrorCollector()
		{ }

		public void Add(int code, string msg)
		{
			Add(new PIDLError() { code = code, msg = msg });
		}

		public void Add(PIDLError err)
		{
			_errors.Add(err);
		}

		List<PIDLError> _errors = new List<PIDLError>();

		public IEnumerable<PIDLError> Errors { get { return _errors; } }

		public void ThrowException(System.Exception inner = null)
		{
			throw new PIDLException(_errors, inner);
		}

	}
}
