/* port: optional launcher metadata; never changes the retail .tsk format. */
#pragma once
namespace neg {
void SavePreviewTick(float dt);
void SavePreviewWritten(const char *name);
void SavePreviewLoad(const char *name);
void SavePreviewNewGame();
void SavePreviewCopy(const char *src, const char *dst);
void SavePreviewDelete(const char *dosPath);
}
