#ifndef pidlCore__basictypes_h
#define pidlCore__basictypes_h

namespace PIDL {

    enum class InvokeStatus
    {
        Ok,
        NotImplemented,
        Error,
        MarshallingError,
        NotSupportedMarshallingVersion,
        FatalError
    };

}

#endif // pidlCore__basictypes_h
