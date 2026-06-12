#include "apemovie_core.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct
{
    const char *output_path;
    const char *load_path;
    const char *ffmpeg_path;
    int load_is_script;
    int quiet_events;
    int days;
    int width;
    int height;
    int fps;
    int minutes_per_frame;
    int frame_limit;
    int bit_rate;
    uint32_t seed;
} apemovie_options;

static void apemovie_usage(FILE *stream)
{
    fprintf(stream,
            "usage: apemovie [options]\n\n"
            "Options:\n"
            "  --output PATH             MP4 output path (default: apemovie.mp4)\n"
            "  --days N                  Simulation days to render (default: 20)\n"
            "  --width N                 Movie width in pixels (default: 2048)\n"
            "  --height N                Movie height in pixels (default: 1536)\n"
            "  --fps N                   Movie frames per second (default: 30)\n"
            "  --minutes-per-frame N     Simulation minutes advanced per frame (default: 5)\n"
            "  --frames N                Override frame count for previews/tests\n"
            "  --seed N                  Random seed, decimal or 0x-prefixed hex\n"
            "  --load PATH               Load a saved simulation before rendering\n"
            "  --script PATH             Load an ApeScript file before rendering\n"
            "  --bitrate N               H.264 average bit rate in bits/sec (default: 12000000)\n"
            "  --ffmpeg PATH             ffmpeg executable path\n"
            "  --events                  Print simulation birth/death event logging\n"
            "  --help                    Show this help\n\n"
            "A 20-day default run advances 28,800 simulation minutes and records one\n"
            "frame every five simulated minutes. Use --minutes-per-frame 1 for a fuller,\n"
            "longer movie, or increase it for faster time-lapse output.\n");
}

static void apemovie_defaults(apemovie_options *options)
{
    options->output_path = "apemovie.mp4";
    options->load_path = 0L;
    options->ffmpeg_path = 0L;
    options->load_is_script = 0;
    options->quiet_events = 1;
    options->days = 20;
    options->width = 2048;
    options->height = 1536;
    options->fps = 30;
    options->minutes_per_frame = 5;
    options->frame_limit = 0;
    options->bit_rate = 12000000;
    options->seed = 0x12738291u;
}

static int apemovie_positive_int(const char *text, const char *name, int *value)
{
    char *end = 0L;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if ((errno != 0) || (end == text) || (end[0] != 0) || (parsed <= 0) || (parsed > 2147483647L))
    {
        fprintf(stderr, "%s must be a positive integer.\n", name);
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

static int apemovie_parse_seed(const char *text, uint32_t *seed)
{
    char *end = 0L;
    unsigned long parsed;
    int base = 10;

    if ((text[0] == '0') && ((text[1] == 'x') || (text[1] == 'X')))
    {
        base = 16;
    }

    errno = 0;
    parsed = strtoul(text, &end, base);
    if ((errno != 0) || (end == text) || (end[0] != 0) || (parsed > 0xffffffffUL))
    {
        fprintf(stderr, "--seed must be a decimal or 0x-prefixed 32-bit integer.\n");
        return 0;
    }

    *seed = (uint32_t)parsed;
    return 1;
}

static int apemovie_parse_options(int argc, char **argv, apemovie_options *options)
{
    int index = 1;

    while (index < argc)
    {
        const char *argument = argv[index];
        const char *value = 0L;

        if ((strcmp(argument, "--help") == 0) || (strcmp(argument, "-h") == 0))
        {
            apemovie_usage(stdout);
            exit(0);
        }

        if ((strcmp(argument, "--events") == 0))
        {
            options->quiet_events = 0;
            index++;
            continue;
        }

        if ((index + 1) >= argc)
        {
            fprintf(stderr, "%s requires a value.\n", argument);
            return 0;
        }

        value = argv[index + 1];

        if (strcmp(argument, "--output") == 0)
        {
            options->output_path = value;
        }
        else if (strcmp(argument, "--days") == 0)
        {
            if (!apemovie_positive_int(value, argument, &options->days)) return 0;
        }
        else if (strcmp(argument, "--width") == 0)
        {
            if (!apemovie_positive_int(value, argument, &options->width)) return 0;
        }
        else if (strcmp(argument, "--height") == 0)
        {
            if (!apemovie_positive_int(value, argument, &options->height)) return 0;
        }
        else if (strcmp(argument, "--fps") == 0)
        {
            if (!apemovie_positive_int(value, argument, &options->fps)) return 0;
        }
        else if (strcmp(argument, "--minutes-per-frame") == 0)
        {
            if (!apemovie_positive_int(value, argument, &options->minutes_per_frame)) return 0;
        }
        else if (strcmp(argument, "--frames") == 0)
        {
            if (!apemovie_positive_int(value, argument, &options->frame_limit)) return 0;
        }
        else if (strcmp(argument, "--bitrate") == 0)
        {
            if (!apemovie_positive_int(value, argument, &options->bit_rate)) return 0;
        }
        else if (strcmp(argument, "--seed") == 0)
        {
            if (!apemovie_parse_seed(value, &options->seed)) return 0;
        }
        else if (strcmp(argument, "--load") == 0)
        {
            options->load_path = value;
            options->load_is_script = 0;
        }
        else if (strcmp(argument, "--script") == 0)
        {
            options->load_path = value;
            options->load_is_script = 1;
        }
        else if (strcmp(argument, "--ffmpeg") == 0)
        {
            options->ffmpeg_path = value;
        }
        else
        {
            fprintf(stderr, "Unknown option: %s\n\n", argument);
            apemovie_usage(stderr);
            return 0;
        }

        index += 2;
    }

    if (((options->width & 1) != 0) || ((options->height & 1) != 0))
    {
        fprintf(stderr, "Movie width and height must be even for H.264 output.\n");
        return 0;
    }

    return 1;
}

static const char *apemovie_find_ffmpeg(const char *requested)
{
    static const char *candidates[] =
    {
        "/opt/homebrew/bin/ffmpeg",
        "/usr/local/bin/ffmpeg",
        "/usr/bin/ffmpeg",
        "ffmpeg",
        0L
    };
    int index = 0;

    if ((requested != 0L) && (requested[0] != 0))
    {
        return requested;
    }

    while (candidates[index] != 0L)
    {
        if ((strchr(candidates[index], '/') == 0) || (access(candidates[index], X_OK) == 0))
        {
            return candidates[index];
        }
        index++;
    }

    return 0L;
}

static char *apemovie_shell_quote(const char *value)
{
    size_t length = 2;
    size_t loop = 0;
    size_t out = 0;
    char *quoted;

    while (value[loop] != 0)
    {
        length += (value[loop] == '\'') ? 4 : 1;
        loop++;
    }

    quoted = (char *)malloc(length + 1);
    if (quoted == 0L)
    {
        return 0L;
    }

    quoted[out++] = '\'';
    loop = 0;
    while (value[loop] != 0)
    {
        if (value[loop] == '\'')
        {
            quoted[out++] = '\'';
            quoted[out++] = '\\';
            quoted[out++] = '\'';
            quoted[out++] = '\'';
        }
        else
        {
            quoted[out++] = value[loop];
        }
        loop++;
    }
    quoted[out++] = '\'';
    quoted[out] = 0;
    return quoted;
}

static char *apemovie_ffmpeg_command(const apemovie_options *options, const char *ffmpeg_path)
{
    char *quoted_ffmpeg = apemovie_shell_quote(ffmpeg_path);
    char *quoted_output = apemovie_shell_quote(options->output_path);
    char *command;
    int needed;

    if ((quoted_ffmpeg == 0L) || (quoted_output == 0L))
    {
        free(quoted_ffmpeg);
        free(quoted_output);
        return 0L;
    }

    needed = snprintf(0L, 0,
                      "%s -y -loglevel error -f rawvideo -pix_fmt bgra -s %dx%d -r %d -i pipe:0 "
                      "-an -c:v libx264 -pix_fmt yuv420p -b:v %d -movflags +faststart %s",
                      quoted_ffmpeg, options->width, options->height, options->fps,
                      options->bit_rate, quoted_output);

    command = (char *)malloc((size_t)needed + 1u);
    if (command != 0L)
    {
        snprintf(command, (size_t)needed + 1u,
                 "%s -y -loglevel error -f rawvideo -pix_fmt bgra -s %dx%d -r %d -i pipe:0 "
                 "-an -c:v libx264 -pix_fmt yuv420p -b:v %d -movflags +faststart %s",
                 quoted_ffmpeg, options->width, options->height, options->fps,
                 options->bit_rate, quoted_output);
    }

    free(quoted_ffmpeg);
    free(quoted_output);
    return command;
}

int main(int argc, char **argv)
{
    apemovie_options options;
    const char *ffmpeg_path;
    char *command;
    FILE *pipe_file;
    uint8_t *frame;
    int frame_count;
    int total_minutes;
    int progress_every;
    int pipe_status;

    apemovie_defaults(&options);

    if (!apemovie_parse_options(argc, argv, &options))
    {
        return 1;
    }

    ffmpeg_path = apemovie_find_ffmpeg(options.ffmpeg_path);
    if (ffmpeg_path == 0L)
    {
        fprintf(stderr, "ffmpeg was not found. Install ffmpeg or pass --ffmpeg PATH.\n");
        return 1;
    }

    total_minutes = options.days * (int)apemovie_minutes_per_day();
    frame_count = (total_minutes + options.minutes_per_frame - 1) / options.minutes_per_frame;
    if (options.frame_limit > 0)
    {
        frame_count = options.frame_limit;
    }
    if (frame_count < 1)
    {
        frame_count = 1;
    }

    frame = (uint8_t *)malloc((size_t)options.width * (size_t)options.height * 4u);
    if (frame == 0L)
    {
        fprintf(stderr, "Unable to allocate frame buffer.\n");
        return 1;
    }

    if (!apemovie_start(options.seed, options.load_path, options.load_is_script, options.quiet_events))
    {
        free(frame);
        fprintf(stderr, "Unable to start the ApeSDK simulation.\n");
        return 1;
    }

    command = apemovie_ffmpeg_command(&options, ffmpeg_path);
    if (command == 0L)
    {
        apemovie_finish();
        free(frame);
        fprintf(stderr, "Unable to build ffmpeg command.\n");
        return 1;
    }

    pipe_file = popen(command, "w");
    free(command);
    if (pipe_file == 0L)
    {
        apemovie_finish();
        free(frame);
        fprintf(stderr, "Unable to start ffmpeg.\n");
        return 1;
    }

    printf("apemovie: writing %d frames to %s\n", frame_count, options.output_path);
    printf("apemovie: %d days, %d simulated minutes/frame, %dx%d @ %d fps\n",
           options.days, options.minutes_per_frame, options.width, options.height, options.fps);

    progress_every = frame_count / 20;
    if (progress_every < 1)
    {
        progress_every = 1;
    }

    for (int frame_index = 0; frame_index < frame_count; frame_index++)
    {
        size_t bytes_to_write = (size_t)options.width * (size_t)options.height * 4u;
        size_t bytes_written;

        if (!apemovie_render_frame_bgra(frame, options.width, options.height, options.width * 4, (uint32_t)options.minutes_per_frame))
        {
            pclose(pipe_file);
            apemovie_finish();
            free(frame);
            fprintf(stderr, "ApeSDK failed to render frame %d.\n", frame_index + 1);
            return 1;
        }

        bytes_written = fwrite(frame, 1, bytes_to_write, pipe_file);
        if (bytes_written != bytes_to_write)
        {
            pclose(pipe_file);
            apemovie_finish();
            free(frame);
            fprintf(stderr, "Unable to write frame %d to ffmpeg.\n", frame_index + 1);
            return 1;
        }

        if ((frame_index == 0) || (frame_index == (frame_count - 1)) || (((frame_index + 1) % progress_every) == 0))
        {
            printf("apemovie: frame %d/%d, simulation day %u, minute %u, beings %d\n",
                   frame_index + 1, frame_count,
                   apemovie_land_date(), apemovie_land_time(), apemovie_being_count());
            fflush(stdout);
        }
    }

    pipe_status = pclose(pipe_file);
    apemovie_finish();
    free(frame);

    if (pipe_status == -1)
    {
        fprintf(stderr, "Unable to close ffmpeg pipe.\n");
        return 1;
    }

    if (WIFEXITED(pipe_status) && (WEXITSTATUS(pipe_status) == 0))
    {
        printf("apemovie: finished %s\n", options.output_path);
        return 0;
    }

    fprintf(stderr, "ffmpeg failed while writing %s.\n", options.output_path);
    return 1;
}
