#include "platform.hpp"

#include "n3ds/audio.hpp"
#include "n3ds/ctru++.hpp"

#include <cassert>
#include <cinttypes>
#include <cstdio>

#include <algorithm>
#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

#include <3ds.h>

#include "def.h"

#include <xmp.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

// --- Poor-man's WAV parser ---
//
// Based on http://soundfile.sapp.org/doc/WaveFormat/
namespace wav {

struct __attribute__((packed)) WavSubchunk1 {
    u32 Subchunk1ID;   // "fmt ", 0x20746d66
    u32 Subchunk1Size; // 16 for PCM
    u16 AudioFormat;   // 1 for PCM
    u16 NumChannels;   // 1 or 2
    u32 SampleRate;    // sample rate
    u32 ByteRate;      // SampleRate * NumChannels * BitsPerSample/8
    u16 BlockAlign;    // NumChannels * BitsPerSample/8
    u16 BitsPerSample; // bits per sample
};

struct __attribute((packed)) WavSubchunk2 {
    u32 Subchunk2ID;   // "data", 0x61746164
    u32 Subchunk2Size; // NumSamples * NumChannels * BitsPerSample/8
    u8 Data[];         // actual sound data.
};

struct __attribute__((packed)) WavFile {
    u32 ChunkID;   // "RIFF", 0x46464952
    u32 ChunkSize; // size of the the rest of the chunk following this field
                   // (file size - 8)
    u32 Format;    // "WAVE", 0x45564157
    WavSubchunk1 subchunk1;
    WavSubchunk2 subchunk2;
};

constexpr u32 MAGIC_CHUNK_ID = 0x46464952;
constexpr u32 MAGIC_FORMAT = 0x45564157;
constexpr u32 MAGIC_SUBCHUNK1_ID = 0x20746d66;
constexpr u32 MAGIC_SUBCHUNK2_ID = 0x61746164;

constexpr u32 SIZE_SUBCHUNK1_PCM = 16;
constexpr u16 AUDIO_FORMAT_PCM = 1;

bool is_valid(const WavFile* wav_file, size_t file_size) {
    if (file_size <= sizeof(WavFile)) {
        return false;
    }

    const WavSubchunk1& subchunk1 = wav_file->subchunk1;
    const WavSubchunk2& subchunk2 = wav_file->subchunk2;

    if (wav_file->ChunkID != MAGIC_CHUNK_ID ||
        wav_file->Format != MAGIC_FORMAT ||
        subchunk1.Subchunk1ID != MAGIC_SUBCHUNK1_ID ||
        subchunk1.Subchunk1Size != SIZE_SUBCHUNK1_PCM ||
        subchunk1.AudioFormat != AUDIO_FORMAT_PCM ||
        subchunk2.Subchunk2ID != MAGIC_SUBCHUNK2_ID) {
        return false;
    }

    if (wav_file->ChunkSize + 8 > file_size) {
        return false;
    }

    if (subchunk1.NumChannels != 1 && subchunk1.NumChannels != 2) {
        return false;
    }

    if (subchunk1.BitsPerSample != 8 && subchunk1.BitsPerSample != 16) {
        return false;
    }

    if (subchunk1.ByteRate != subchunk1.SampleRate * subchunk1.NumChannels *
                                  (subchunk1.BitsPerSample / 8)) {
        return false;
    }

    if (subchunk1.BlockAlign !=
        subchunk1.NumChannels * (subchunk1.BitsPerSample / 8)) {
        return false;
    }

    if (subchunk2.Subchunk2Size + sizeof(WavFile) > file_size) {
        return false;
    }

    return true;
}

} /* namespace wav */

class Sfx {
    size_t m_size{};
    ctru::linear::ptr<const u8[]> m_data{};

    wav::WavSubchunk1 m_wav_info;

public:
    bool load(std::filesystem::path path) {
#ifdef TRACY_ENABLE
        ZoneScopedC(tracy::Color::Lime);
        auto filename = path.filename().string();
        ZoneText(filename.c_str(), filename.size());
#endif

        std::fstream fs(path, std::ios::binary | std::ios::in);
        if (!fs.is_open()) {
            return false;
        }

        fs.seekg(0, std::ios::end);
        auto len = fs.tellg();
        if (len <= sizeof(wav::WavFile)) {
            return false;
        }
        fs.seekg(0, std::ios::beg);

        std::vector<u8> file_data(len);
        fs.read((char*)file_data.data(), len);
        if (fs.gcount() != len) {
            printf(
                "Failed to read file %s (expected size: %d, actual size: %d)\n",
                path.c_str(), (int)len, fs.gcount());
            return false;
        }

        const wav::WavFile* wav_file = (const wav::WavFile*)file_data.data();
        if (!wav::is_valid(wav_file, len)) {
            return false;
        }

        size_t audio_size = wav_file->subchunk2.Subchunk2Size;

        ctru::linear::ptr<u8[]> audio_data =
            ctru::linear::make_ptr_nothrow_for_overwrite<u8[]>(audio_size);

        memcpy(audio_data.get(), wav_file->subchunk2.Data, audio_size);

        this->m_size = audio_size;
        this->m_data = std::move(audio_data);
        this->m_wav_info = wav_file->subchunk1;
        return true;
    }

    inline void unload() {
        m_size = 0;
        m_data.reset();
        m_wav_info = {};
    }

    constexpr operator bool() const { return m_size != 0 && m_data; }

    constexpr u16 num_channels() const { return m_wav_info.NumChannels; }

    constexpr u32 sample_rate() const { return m_wav_info.SampleRate; }

    constexpr u16 bits_per_sample() const { return m_wav_info.BitsPerSample; }

    inline std::pair<const u8*, size_t> audio_data() const {
        return {m_data.get(), m_size};
    }
};

class MusicMod {
    size_t m_size{};
    std::unique_ptr<const u8[]> m_data{};

public:
    bool load(std::filesystem::path path) {
#ifdef TRACY_ENABLE
        ZoneScopedC(tracy::Color::Lime);
        auto filename = path.filename().string();
        ZoneText(filename.c_str(), filename.size());
#endif

        std::fstream fs(path, std::ios::binary | std::ios::in);
        if (!fs.is_open()) {
            return false;
        }

        fs.seekg(0, std::ios::end);
        auto len = fs.tellg();
        if (len <= 0) {
            return false;
        }
        fs.seekg(0, std::ios::beg);

        std::unique_ptr<u8[]> file_data = std::make_unique<u8[]>(len);
        fs.read((char*)file_data.get(), len);
        if (fs.gcount() != len) {
            printf(
                "Failed to read file %s (expected size: %d, actual size: %d)\n",
                path.c_str(), (int)len, fs.gcount());
            return false;
        }

        if (xmp_test_module_from_memory(file_data.get(), len, nullptr) != 0) {
            fprintf(stderr, "File '%s' is not a valid music file module.\n",
                    path.c_str());
            return false;
        }

        this->m_size = len;
        this->m_data = std::move(file_data);
        return true;
    }

    inline void unload() {
        m_size = 0;
        m_data.reset();
    }

    constexpr operator bool() const { return m_size != 0 && m_data; }

    inline std::pair<const u8*, size_t> module_data() const {
        return {m_data.get(), m_size};
    }
};

class NdspAudioThread {
    // The inter-thread communication state. Only the data in this struct
    // should be touched from any non-audio threads.
    struct State {
        Thread thread;

        LightEvent in_event{};
        std::atomic<bool> in_run{};

        std::atomic<bool> in_stop_song{};
        std::atomic<bool> in_start_song{};
        int in_start_song_arg_song_id{};
        bool in_start_song_arg_loop{};

        std::atomic<float> in_music_tempo_factor{};
        static_assert(std::atomic<float>::is_always_lock_free);

        std::atomic<bool> out_song_ended{};
    };
    State m_state;

    // The following fields must only be accessed from the audio thread.

    xmp_context m_xmp_ctx{};
    bool m_is_playing{false};
    float m_music_tempo_factor{1.f};
    int m_loop_count{};

    void audio_thread();
    static void audio_thread_start(void* ctx) {
        ((NdspAudioThread*)ctx)->audio_thread();
    }

    bool start_song(int song_id, bool loop);
    void stop_song();

    bool fill_buffer();

    static void ndsp_callback(void* ctx);

    NdspAudioThread() = default;
    ~NdspAudioThread() = default;

    NdspAudioThread(const NdspAudioThread&) = delete;
    NdspAudioThread(NdspAudioThread&&) = delete;
    auto operator=(const NdspAudioThread&) = delete;
    auto operator=(NdspAudioThread&&) = delete;

    static std::atomic<NdspAudioThread*> s_instance;

public:
    static bool start_thread();
    static void join_thread();

    static void send_start_song(int song_id, bool loop);
    static void send_stop_song();
    static void send_set_tempo(float tempo_factor);

    static bool recv_song_ended();
};

std::atomic<NdspAudioThread*> NdspAudioThread::s_instance{};

enum class SoundEngine {
    NONE,
    NDSP, // main audio support
    CSND, // provides rudimentary sfx as a fallback
};

enum ChannelsCSND : int {
    CH_CSND_SFX = 8,
};

enum ChannelsNDSP : int {
    CH_NDSP_SFX_FIRST = 0,
    CH_NDSP_SFX_LAST =
        CH_NDSP_SFX_FIRST + 8 - 1, // 8 channels for sfx ought to be enough???
    CH_NDSP_MUSIC,
};

constexpr int MUSIC_SAMPLING_RATE =
    NDSP_SAMPLE_RATE; // native 3DS DSP sampling rate.
constexpr int MUSIC_NUM_CHANNELS = 2;
constexpr int MUSIC_BITS_PER_SAMPLE = 16;
// 0.1-second buffer
constexpr size_t MUSIC_BUFFER_SAMPLE_COUNT = MUSIC_SAMPLING_RATE * 1 / 10;
constexpr size_t MUSIC_BUFFER_SIZE = MUSIC_BUFFER_SAMPLE_COUNT *
                                     MUSIC_NUM_CHANNELS *
                                     (MUSIC_BITS_PER_SAMPLE / 8);

static SoundEngine sound_engine = SoundEngine::NONE;

static std::array<Sfx, SFX_COUNT> sound_effects;

static std::array<ndspWaveBuf, CH_NDSP_SFX_LAST - CH_NDSP_SFX_FIRST + 1>
    ndsp_wave_buf_sfx{};
static std::list<int> sfx_use_order{};

// Buffers for storing the music output, two of them to allow flipping
// (therefore size is `MUSIC_BUFFER_SIZE * 2`).
static ctru::linear::ptr<u8[]> music_audio_buf{};
static std::array<ndspWaveBuf, 2> ndsp_wave_buf_music{};
static int music_next_buf_idx = 0;

static float music_volume = 1.f;

static std::vector<MusicMod> song_modules{};

Songs songs;

inline float sfx_volume() { return savefile->settings.sfxVolume / 10.f; }

// Game engine functions:

void loadAudio(std::string pathPrefix) {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    std::filesystem::path prefix{pathPrefix};

    std::filesystem::path assets_path{prefix / "assets"};
    std::error_code ec;
    std::filesystem::directory_iterator dir_it{assets_path, ec};
    if (ec) {
        errorConf err;
        errorInit(&err, ERROR_TEXT_WORD_WRAP, CFG_LANGUAGE_EN);
        errorText(&err,
                  "Failed to load audio assets. Please make sure the files has "
                  "been placed on the SD card under `" DATA_DIR_BASE
                  "/assets`.");
        errorDisp(&err);
        return;
    }

    Result res = ndspInit();
    if (R_FAILED(res)) {
        if (R_DESCRIPTION(res) == RD_NOT_FOUND) {
            fprintf(stderr, "Failed to initialize DSP, firmware not found.\n");
        } else {
            fprintf(stderr, "Failed to initialize DSP, err: 0x%" PRIx32 "\n",
                    res);
        }
        errorConf err;
        errorInit(&err, ERROR_TEXT_WORD_WRAP, CFG_LANGUAGE_EN);
        errorText(
            &err,
            "Failed to initialize DSP. Game audio will be limited. Please make "
            "sure you have the DSP firmware available.\n\nFor more "
            "information, see https://3ds.hacks.guide/finalizing-setup.html");
        errorDisp(&err);

        Result res = csndInit();
        if (R_FAILED(res)) {
            errorInit(&err, ERROR_TEXT_WORD_WRAP, CFG_LANGUAGE_EN);
            errorText(&err, "Failed to initialize CSND. Game audio will be "
                            "fully disabled.");
            errorDisp(&err);
            return;
        }

        sound_engine = SoundEngine::CSND;
    } else {
        sound_engine = SoundEngine::NDSP;
    }

    int failed_sfx_count = 0;
    for (int i = 0; i < SFX_COUNT; i++) {
        auto& sfx = sound_effects[i];
        auto file = prefix / std::string{SoundEffectPaths[i]};
        if (!sfx.load(file.c_str())) {
            failed_sfx_count++;
            fprintf(stderr, "Failed to load sfx '%s'\n", file.c_str());
            continue;
        }

        fprintf(stderr,
                "Loaded sfx '%s' - channels: %d, sample rate: %d, bits per "
                "sample: %d\n",
                file.c_str(), sfx.num_channels(), (int)sfx.sample_rate(),
                sfx.bits_per_sample());
    }

    if (failed_sfx_count > 0) {
        errorConf err;
        errorInit(&err, ERROR_TEXT_WORD_WRAP, CFG_LANGUAGE_EN);
        char buf[256];
        snprintf(buf, 256,
                 "Failed to load %d sfx audio assets. Please make sure the "
                 "files has been placed on the SD card under `" DATA_DIR_BASE
                 "/assets`.",
                 failed_sfx_count);
        errorText(&err, buf);
        errorDisp(&err);
    }

    if (sound_engine != SoundEngine::NDSP) {
        return;
    }

    // NDSP-specific initialization:

    ndspSetOutputMode(NDSP_OUTPUT_STEREO);

    for (int i = CH_NDSP_SFX_FIRST; i <= CH_NDSP_SFX_LAST; i++) {
        auto& wave_buf = ndsp_wave_buf_sfx[i - CH_NDSP_SFX_FIRST];
        wave_buf.status = NDSP_WBUF_FREE;
        wave_buf.looping = false;

        ndspChnInitParams(i);
        ndspChnSetInterp(i, NDSP_INTERP_POLYPHASE);

        sfx_use_order.push_back(i);
    }

    ndspChnInitParams(CH_NDSP_MUSIC);
    ndspChnSetInterp(CH_NDSP_MUSIC, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(CH_NDSP_MUSIC, MUSIC_SAMPLING_RATE);
    ndspChnSetFormat(CH_NDSP_MUSIC, NDSP_FORMAT_STEREO_PCM16);

    music_audio_buf =
        ctru::linear::make_ptr_nothrow<u8[]>(MUSIC_BUFFER_SIZE * 2);
    for (auto& wave_buf : ndsp_wave_buf_music) {
        wave_buf.status = NDSP_WBUF_FREE;
        wave_buf.looping = false;
        wave_buf.nsamples = MUSIC_BUFFER_SAMPLE_COUNT;
    }

    for (const auto& entry : dir_it) {
        if (!entry.is_regular_file()) {
            continue;
        }

        for (const auto& ext : {".it", ".mod", ".xm"}) {
            if (entry.path().extension() == ext) {
                MusicMod mod;
                if (!mod.load(entry.path())) {
                    continue;
                }

                auto name = entry.path().filename().string();
                if (name.find("MENU_") != std::string::npos) {
                    songs.menu.push_back(song_modules.size());
                } else {
                    songs.game.push_back(song_modules.size());
                }
                fprintf(stderr, "Found music '%s'\n", entry.path().c_str());
                song_modules.push_back(std::move(mod));
            }
        }
    }

    if (!NdspAudioThread::start_thread()) {
        fprintf(stderr, "Failed to start audio thread\n");
        ndspExit();
        sound_engine = SoundEngine::NONE;
    }
}

void freeAudio() {
    if (sound_engine == SoundEngine::NDSP) {
        NdspAudioThread::join_thread();

        for (int i = CH_NDSP_SFX_FIRST; i <= CH_NDSP_SFX_LAST; i++) {
            ndspChnWaveBufClear(i);
        }
        ndspChnWaveBufClear(CH_NDSP_MUSIC);
    }

    music_audio_buf.reset();

    songs.menu.clear();
    songs.game.clear();
    song_modules.clear();

    for (auto& sfx : sound_effects) {
        sfx.unload();
    }

    switch (sound_engine) {
    case SoundEngine::NDSP:
        ndspExit();
        break;
    case SoundEngine::CSND:
        csndExit();
        break;
    case SoundEngine::NONE:
        break;
    }

    sound_engine = SoundEngine::NONE;
}

void audioProcess() {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    if (sound_engine != SoundEngine::NDSP) {
        return;
    }

    if (music_volume < 1e-6f) {
        return;
    }

    if (NdspAudioThread::recv_song_ended()) {
        // printf("Song ended.\n");
        if (savefile->settings.cycleSongs == 1) { // CYCLE
            playNextSong();
        } else if (savefile->settings.cycleSongs == 2) { // SHUFFLE
            playSongRandom(currentMenu);
        }
    }
}

void sfx(int n) { sfxRate(n, 1.0); }

void sfxRate(int n, float playback_rate) {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    const float volume = sfx_volume();
    if (volume < 1e-6) {
        return;
    }

    const auto& sfx = sound_effects[n];
    if (!sfx) {
        return;
    }

    if (playback_rate <= 0.01) {
        playback_rate = 0.01;
    }

    auto [audio_data, audio_size] = sfx.audio_data();

    switch (sound_engine) {

    case SoundEngine::NDSP: {
        auto use_ch_it = std::find_if(
            sfx_use_order.cbegin(), sfx_use_order.cend(), [](int ch) {
                const auto& wave_buf =
                    ndsp_wave_buf_sfx[ch - CH_NDSP_SFX_FIRST];
                // bool playing = ndspChnIsPlaying(use_ch);
                // printf("channel %d playing: %d, status: %d\n", use_ch,
                // playing, wave_buf.status);
                if (wave_buf.status != NDSP_WBUF_QUEUED &&
                    wave_buf.status != NDSP_WBUF_PLAYING) {
                    return true;
                }
                return false;
            });
        if (use_ch_it == sfx_use_order.cend()) {
            use_ch_it = sfx_use_order.cbegin();
            fprintf(stderr,
                    " No free channel left for sfx %d, replacing channel %d\n",
                    n, *use_ch_it);
            ndspChnWaveBufClear(*use_ch_it);
        }
        int use_ch = *use_ch_it;
        sfx_use_order.splice(sfx_use_order.cend(), sfx_use_order, use_ch_it);
        // printf("Playing sfx %d on channel %d\n", n, use_ch);

        ndspChnSetRate(use_ch, sfx.sample_rate() * playback_rate);

        if (sfx.num_channels() == 1 && sfx.bits_per_sample() == 8) {
            ndspChnSetFormat(use_ch, NDSP_FORMAT_MONO_PCM8);
        } else if (sfx.num_channels() == 1 && sfx.bits_per_sample() == 16) {
            ndspChnSetFormat(use_ch, NDSP_FORMAT_MONO_PCM16);
        } else if (sfx.num_channels() == 2 && sfx.bits_per_sample() == 8) {
            ndspChnSetFormat(use_ch, NDSP_FORMAT_STEREO_PCM8);
        } else if (sfx.num_channels() == 2 && sfx.bits_per_sample() == 16) {
            ndspChnSetFormat(use_ch, NDSP_FORMAT_STEREO_PCM16);
        } else {
            assert(false && "Unsupported SFX format");
            return;
        }

        float mix[12]{volume, volume};
        ndspChnSetMix(use_ch, mix);

        auto& wave_buf = ndsp_wave_buf_sfx[use_ch - CH_NDSP_SFX_FIRST];
        wave_buf.data_vaddr = audio_data;
        wave_buf.looping = false;
        wave_buf.nsamples =
            audio_size / (sfx.num_channels() * (sfx.bits_per_sample() / 8));
        // The ndspWaveBuf must have static lifetime.
        ndspChnWaveBufAdd(use_ch, &wave_buf);
    } break;

    case SoundEngine::CSND: {
        u32 flags = SOUND_CHANNEL(sfx.num_channels()) | SOUND_ONE_SHOT;
        switch (sfx.bits_per_sample()) {
        case 8:
            flags |= SOUND_FORMAT_8BIT;
            break;
        case 16:
            flags |= SOUND_FORMAT_16BIT;
            break;
        default:
            return;
        }
        csndPlaySound(CH_CSND_SFX, flags, sfx.sample_rate() * playback_rate,
                      volume, 0.0, (void*)audio_data, (void*)audio_data,
                      audio_size);
    } break;

    case SoundEngine::NONE:
        break;
    }
}

void startSong(int song, bool loop) {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    if (sound_engine != SoundEngine::NDSP) {
        return;
    }

    if (ndspChnIsPaused(CH_NDSP_MUSIC)) {
        ndspChnWaveBufClear(CH_NDSP_MUSIC);
        ndspChnSetPaused(CH_NDSP_MUSIC, false);
    }

    NdspAudioThread::send_start_song(song, loop);
}

void stopSong() {
    if (sound_engine != SoundEngine::NDSP) {
        return;
    }

    NdspAudioThread::send_stop_song();
}

void pauseSong() {
    if (sound_engine != SoundEngine::NDSP) {
        return;
    }

    ndspChnSetPaused(CH_NDSP_MUSIC, true);
}

void resumeSong() {
    if (sound_engine != SoundEngine::NDSP) {
        return;
    }

    ndspChnSetPaused(CH_NDSP_MUSIC, false);
}

void setMusicVolume(int volume) {
    if (sound_engine != SoundEngine::NDSP) {
        return;
    }

    music_volume = volume * (1.f / 512.f);

    float mix[12]{music_volume, music_volume};
    ndspChnSetMix(CH_NDSP_MUSIC, mix);
}

void setMusicTempo(int tempo) {
    if (sound_engine != SoundEngine::NDSP) {
        return;
    }

    if (tempo < 1) {
        tempo = 1;
    }

    const float playback_rate = tempo * (1.f / 1024.f);
    const float music_tempo_factor = 1.f / playback_rate;

    NdspAudioThread::send_set_tempo(music_tempo_factor);
}

// Audio thread functions:

bool NdspAudioThread::start_thread() {
    // This is called on the main thread.
    assert(!s_instance.load());

    auto* inst = new NdspAudioThread;
    s_instance.store(inst, std::memory_order_release);

    inst->m_state.in_run.store(true);
    LightEvent_Init(&inst->m_state.in_event, RESET_ONESHOT);

    s32 priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    // Make sure our audio thread has lower priority than the main thread.
    priority += 1;
    priority = std::min(std::max(priority, (s32)0x18), (s32)0x3f);

    // APT_SetAppCpuTimeLimit(20);
    inst->m_state.thread =
        threadCreate(audio_thread_start, inst, 64 * 1024, priority, -1, false);
    if (!inst->m_state.thread) {
        fprintf(stderr, "Failed to start audio thread\n");
        return false;
    }

    return true;
}

void NdspAudioThread::join_thread() {
    // This is called on the main thread.
    auto* inst = s_instance.exchange(nullptr, std::memory_order_acq_rel);
    assert(inst);

    inst->m_state.in_run.store(false, std::memory_order_release);
    LightEvent_Signal(&inst->m_state.in_event);
    threadJoin(inst->m_state.thread, U64_MAX);
    threadFree(inst->m_state.thread);
    delete inst;
}

void NdspAudioThread::send_start_song(int song_id, bool loop) {
    // This is called on the main thread.
    auto* inst = s_instance.load(std::memory_order_relaxed);
    assert(inst);

    if (inst->m_state.in_start_song.exchange(false,
                                             std::memory_order_acq_rel)) {
        fprintf(stderr,
                "NdspAudioThread::send_start_song possible race condition\n");
    }
    if (inst->m_state.in_stop_song.exchange(false, std::memory_order_acq_rel)) {
        fprintf(stderr, "send_start_song Cancelled song stop request\n");
    }
    inst->m_state.in_start_song_arg_song_id = song_id;
    inst->m_state.in_start_song_arg_loop = loop;
    inst->m_state.in_start_song.store(true, std::memory_order_release);
    inst->m_state.out_song_ended.store(false, std::memory_order_relaxed);
}

void NdspAudioThread::send_stop_song() {
    // This is called on the main thread.
    auto* inst = s_instance.load(std::memory_order_relaxed);
    assert(inst);

    if (inst->m_state.in_start_song.exchange(false,
                                             std::memory_order_acq_rel)) {
        fprintf(stderr, "send_stop_song cancelled song start request\n");
    }
    inst->m_state.in_stop_song.store(true, std::memory_order_release);
    inst->m_state.out_song_ended.store(false, std::memory_order_relaxed);
}

void NdspAudioThread::send_set_tempo(float tempo_factor) {
    // This is called on the main thread.
    auto* inst = s_instance.load(std::memory_order_relaxed);
    assert(inst);

    inst->m_state.in_music_tempo_factor.store(tempo_factor,
                                              std::memory_order_release);
}

bool NdspAudioThread::recv_song_ended() {
    // This is called on the main thread.
    auto* inst = s_instance.load(std::memory_order_relaxed);
    assert(inst);

    return inst->m_state.out_song_ended.exchange(false,
                                                 std::memory_order_acq_rel);
}

void NdspAudioThread::ndsp_callback(void* ctx) {
    // This is called on the libctru NDSP thread.
    if (NdspAudioThread::s_instance.load(std::memory_order_consume) != ctx) {
        return;
    }
    auto* inst = (NdspAudioThread*)ctx;
    LightEvent_Signal(&inst->m_state.in_event);
}

void NdspAudioThread::audio_thread() {
#ifdef TRACY_ENABLE
    tracy::SetThreadName("Ndsp audio thread");
#endif

    m_xmp_ctx = xmp_create_context();

    ndspSetCallback(ndsp_callback, this);

    while (m_state.in_run.load(std::memory_order_consume)) {

        if (m_state.in_stop_song.exchange(false, std::memory_order_acq_rel)) {
            stop_song();
            // Do not set m_state.out_song_ended
        }

        if (m_state.in_start_song.load(std::memory_order_acquire)) {
            int song_id = m_state.in_start_song_arg_song_id;
            bool loop = m_state.in_start_song_arg_loop;
            if (m_state.in_start_song.exchange(false,
                                               std::memory_order_acq_rel)) {
                if (start_song(song_id, loop)) {
                    m_state.out_song_ended.store(false,
                                                 std::memory_order_release);
                } else {
                    m_state.out_song_ended.store(true,
                                                 std::memory_order_release);
                }
            } else {
                fprintf(stderr,
                        "NdspAudioThread: new song request cancelled\n");
            }
        }

        if (m_is_playing) {
            float new_tempo_factor =
                m_state.in_music_tempo_factor.load(std::memory_order_consume);
            if (new_tempo_factor != m_music_tempo_factor) {
                m_music_tempo_factor = new_tempo_factor;
                xmp_set_tempo_factor(m_xmp_ctx, m_music_tempo_factor);
            }
        }

        if (m_is_playing) {
            if (!fill_buffer()) {
                // printf("Song ended.\n");
                m_state.out_song_ended.store(true, std::memory_order_release);
            }
        }

        LightEvent_Wait(&m_state.in_event);
    }

    ndspSetCallback(nullptr, nullptr);

    stop_song();
    xmp_free_context(m_xmp_ctx);
}

bool NdspAudioThread::start_song(int song_id, bool loop) {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    stop_song();

    auto& song_mod = song_modules[song_id];
    if (!song_mod) {
        return false;
    }

    auto [mod_data, mod_size] = song_mod.module_data();

    if (xmp_load_module_from_memory(m_xmp_ctx, mod_data, mod_size) != 0) {
        fprintf(stderr, "Failed to load song module %d\n", song_id);
        song_mod.unload();
        return false;
    }

    if (xmp_start_player(m_xmp_ctx, MUSIC_SAMPLING_RATE, 0) != 0) {
        fprintf(stderr, "Failed to start player to play song module %d\n",
                song_id);
        xmp_stop_module(m_xmp_ctx);
        song_mod.unload();
        return false;
    }

    xmp_set_tempo_factor(m_xmp_ctx, m_music_tempo_factor);

    // printf("Playing song module %d\n", song_id);
    m_is_playing = true;
    m_loop_count = loop ? 0 : 1;

    return true;
}

void NdspAudioThread::stop_song() {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    ndspChnWaveBufClear(CH_NDSP_MUSIC);

    if (m_is_playing) {
        // printf("Stopping song module\n");
        xmp_end_player(m_xmp_ctx);
        xmp_stop_module(m_xmp_ctx);
        m_is_playing = false;
    }
}

bool NdspAudioThread::fill_buffer() {
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    const auto status = ndsp_wave_buf_music[music_next_buf_idx].status;
    if (status == NDSP_WBUF_QUEUED || status == NDSP_WBUF_PLAYING) {
        // Buffers already full.
        return true;
    }

#ifdef TRACY_ENABLE
    ZoneNamedN(zonefill, "Filling buffer", true);
    ZoneColorV(zonefill,
               music_next_buf_idx ? tracy::Color::Pink : tracy::Color::Lime);
#endif

    u8* buffer = music_audio_buf.get() + music_next_buf_idx * MUSIC_BUFFER_SIZE;
    auto& wave_buf = ndsp_wave_buf_music[music_next_buf_idx];
    wave_buf.data_vaddr = buffer;

    auto res =
        xmp_play_buffer(m_xmp_ctx, buffer, MUSIC_BUFFER_SIZE, m_loop_count);
    if (res == -XMP_END) {
        // Play next song
        return false;
    } else if (res != 0) {
        fprintf(stderr, "Failed to continue playing song.\n");
        return false;
    }

    DSP_FlushDataCache(buffer, MUSIC_BUFFER_SIZE);

    ndspChnWaveBufAdd(CH_NDSP_MUSIC, &wave_buf);

    music_next_buf_idx = (music_next_buf_idx + 1) % 2;

    return true;
}
