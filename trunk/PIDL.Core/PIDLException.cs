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
