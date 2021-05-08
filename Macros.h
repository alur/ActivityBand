#pragma once

// Deletes the given object if it's not a nullptr, and then sets the pointer to nullptr.
#define SAFE_DELETE(obj) if (obj != nullptr) { delete obj; obj = nullptr; }
#define SAFE_RELEASE(x) if (x != nullptr) { (x)->Release(); x = nullptr; }
#define SAFE_FREE(x) if (x != nullptr) { free(x); x = nullptr; }

#define CHECK_FLAG(flags, flag) (((flags) & (flag)) == flag)