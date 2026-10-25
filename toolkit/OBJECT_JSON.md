# Document-owned JSON

`object.c` provides an additive document API declared in `object_json.h`. The
existing `n_object`/`n_array` API and its ownership rules remain available.
Document values embed an `n_object` model, decode UTF-8 strings and Unicode
escapes, retain string/key byte lengths, and preserve signed and unsigned 64-bit
integers on native and WebAssembly targets.

Read with `object_json_read`, inspect with the `object_json_*` accessors, and free
the complete document with `object_json_doc_free`. Mutable documents own all
their values, strings and keys. Adding a value already attached to a container
copies it, so one container cannot change another container's sibling chain.
Use the length-aware accessors for embedded NUL bytes.

Document values must use document accessors and document destruction. Do not
pass them to legacy `obj_free`, `unknown_free`, or legacy numeric/string
accessors: decoded strings and full-width numbers have different representation
and ownership requirements.

The strict reader rejects malformed UTF-8, invalid surrogate pairs, invalid
number grammar, non-finite numbers, trailing data and nesting beyond 128 levels.
Input and serialized output are bounded to 32 MiB. An optional allocator supports
host arenas and allocation-failure checks. Defining `APESDK_OBJECT_JSON_ONLY`
builds this API from `object.c` without linking the legacy toolkit runtime.

Run `sh toolkit/test/test_object_document.sh` from the SDK root for document
tests. The existing object/string/file/vector, strict JSON, allocation-failure
and fuzz tests remain unchanged in `toolkit/test/`.
