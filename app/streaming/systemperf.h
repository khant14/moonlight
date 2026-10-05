#pragma once

// OS-level tweaks applied for the duration of a stream to keep the system
// from treating Moonlight as low-priority background work.
namespace SystemPerf {

// Call when the stream window is about to be shown
void beginStreaming();

// Call once the stream has ended
void endStreaming();

}
