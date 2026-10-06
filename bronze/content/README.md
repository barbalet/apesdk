# Bronze scenario content

`first_slice.bronze` is the initial integration fixture: farmer, fisher, and
bronze smith; grain/fish food; copper/tin/charcoal production; and one shared
settlement policy.  It is intentionally small enough to run as a deterministic
apeSDK adapter test.

`bronze_age_full.bronze` is the complete 63-vocation scenario.  The package
runtime builds it directly with `make -C bronze test`.
