#pragma once

#if RHI_ENABLE_PROFILER


#else

#define RHI_FRAME_MARK
#define RHI_FRAME_MARK_NAMED(name)
#define RHI_FRAME_MARK_NAMED_COLORED(name, colored)

#define RHI_PROFILE_SCOPE
#define RHI_PROFILE_SCOPE_NAMED(name)
#define RHI_PROFILE_SCOPE_NAMED_COLORED(name, color)

#endif