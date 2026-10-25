# INI Parser
Very rudimentary parser for .ini and .conf files.

# Usage
All usage revolves around 2 structures and 2 functions; `ini_parser` is the core
state of the parser, and `ini_callbacks` contains callbacks that the parser uses
to output parsed data. To initialize `ini_parser`, zero it out. Since it uses no
heap memory, there is no need to deinitialize it after use. The two functions
that run the parser are `ini_parser_feed` and `ini_parser_finish`, the former of
which is used to feed the parser data from the configuration file being parsed,
and the latter of which is used to finalize the parsing once there isn't any
more data to be fed. Both of these take an `ini_callbacks` structure which lets
the user retrieve parsed data. There are two callbacks, `on_section` and
`on_pair`, both of which must be defined, and both of which take a `user`
pointer that may be used for arbitrary data; they are expected to return 0,
otherwise the parser will halt and return a non-zero value to indicate an error.
