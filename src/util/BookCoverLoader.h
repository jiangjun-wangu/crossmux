#pragma once

#include <string>

namespace BookCoverLoader {

std::string ensureThumbnail(const std::string& bookPath, int height, bool* generated = nullptr);
std::string ensureFullCover(const std::string& bookPath, std::string* title = nullptr, std::string* author = nullptr,
                            bool* generated = nullptr);

// Returns true only when `path` exists and contains a well-formed BMP whose
// declared size matches (or is smaller than) the actual file size. Truncated
// caches written by a failed generation step are rejected so callers can skip
// them and fall back to placeholders instead of failing mid-render.
bool isValidBmp(const std::string& path);

}  // namespace BookCoverLoader
