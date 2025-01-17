#ifndef PROJECT3_TEST_AUDIOSOURCE_LOOPING_HPP
#define PROJECT3_TEST_AUDIOSOURCE_LOOPING_HPP

#include <audio/source_types/iaudiosource.hpp>

namespace audio {
    struct AudioSourceLooping final : IAudioSource {
        ~AudioSourceLooping() override = default;
        void bind(IXAudio2* interface, const std::string& name) override;
        void start(int operation_set) override;
        void stop(int operation_set) override;
        void pause() override;
        void set_volume(float f) override;
        NODISCARD SourceType type() CNOEXCEPT override;

    private:
        AudioVoiceSource _play_source;
        bool _playing = false;
    };
}

#endif //PROJECT3_TEST_AUDIOSOURCE_LOOPING_HPP
