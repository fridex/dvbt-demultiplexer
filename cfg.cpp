/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#include "cfg.h"

#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <algorithm>

#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>

#include <bass.h>

#include "ts.h"

/**
 * Get output filename
 * @param name name of ts file
 * @return true on success
 */
bool Cfg::filename(std::string & name) {
    // create dir name
    if (name[name.size() - 3] == '.'
            && name[name.size() - 2] == 't'
            && name[name.size() - 1] == 's') {
        m_dir_name = name.substr(0, name.size() - 3);
    } else {
        m_dir_name = name + "_out";
        WARN << "Input file does not end with .ts!\n";
        WARN << "Output will be written to " << m_dir_name << std::endl;
    }

    if (mkdir(m_dir_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
        ERROR << m_dir_name << ":"
                  << strerror(errno) << std::endl;
        return false;
    }

    TSDBG << "Created output dir: " << m_dir_name << std::endl;

    return true;
}

/**
 * Open info.txt file
 *
 * @return true on success
 */
bool Cfg::open_info() {
    std::string fname = m_dir_name + "/info.txt";
    info.open(fname);

    if (! info.is_open()) {
        ERROR << fname << ":"
                 << strerror(errno) << std::endl;
        TSDBG << std::endl;
        return false;
    } else
        return true;
}

/**
 * Search PAT table for PID
 * @param pid a PID to search for
 * @return true if PID was found
 */
bool Cfg::search_pat(unsigned pid) {
    for (auto it = programs.begin(); it != programs.end(); ++it)
        if ((*it).second == pid)
            return true;

    return false;
}

/**
 * Search audio for PID
 *
 * @param pid a PID to search for
 * @return true if PID was found
 */
bool Cfg::audios_find(unsigned pid) {
    for (auto it = audios.begin(); it != audios.end(); ++it)
        if ((*it).first == pid)
            return true;

    return false;

}

/**
 * Search video for PID
 *
 * @param pid a PID to search for
 * @return true if PID was found
 */
bool Cfg::videos_find(unsigned pid) {
    for (auto it = videos.begin(); it != videos.end(); ++it)
        if ((*it).first == pid)
            return true;

    return false;
}

/**
 * Search audio for PID
 *
 * @param pid a PID to search for
 * @param res result to store
 * @return true if PID was found
 */
bool Cfg::audios_find_pid(unsigned pid, unsigned * res) {
    for (auto it = audios.begin(); it != audios.end(); ++it) {
        if ((*it).second == pid) {
            if (res)
                *res = (*it).first;
            return true;
        }
    }

    return false;
}

/**
 * Search audio for PID
 *
 * @param pid a PID to search for
 * @param res result to store
 * @return true if PID was found

 */
bool Cfg::videos_find_pid(unsigned pid, unsigned * res) {
    for (auto it = videos.begin(); it != videos.end(); ++it) {
        if ((*it).second == pid) {
            if (res)
                *res = (*it).first;
            return true;
        }
    }

    return false;
}

/**
 * Convert from MJD
 * @param nMJD MJD representation
 * @param nDay output
 * @param nMonth output
 * @param nYear output
 * @return void
 */
void Cfg::ConvertFromMJD(int nMJD, int &nDay, int &nMonth, int &nYear) {
        int Y1 = (int)((nMJD - 15078.2) / 365.25);
        int M1 = (int)((nMJD - 14956.1 - (int)(Y1 * 365.25)) / 30.6001);

        nDay = nMJD - 14956 - (int)(Y1 * 365.25) - (int)(M1 * 30.6001);

        int K = 0;

        if (M1 == 14 || M1 == 15) {
                K = 1;
        } else {
                K = 0;
        }

        nYear = Y1 + K + 1900;
        nMonth = M1 - 1 - K * 12;
}

/**
 * Compare lhs and rhs
 */
template<class T>
struct less_first
: std::binary_function<T,T,bool>
{
   inline bool operator()(const T& lhs, const T& rhs)
   {
      return lhs.second.first.first < rhs.second.first.first;
   }
};

/**
 * Increment day
 * @param day day to increment
 * @param month month
 * @param year year
 * @return void
 */
static inline void incDay(int & day, int & month, int & year) {
    day++;

    switch (month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
            // 31 days in month
            if (day == 32) {
                day = 1;
                month++;
            }
            break;
        case 12:
            // 31 days in month, end of year! Happy new year, Josh!!!
            if (day == 32) {
                day = 1;
                month = 1;
                year++;
            }
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            // 30 days in month
            if (day == 32) {
                day = 1;
                month++;
            }
            break;
        case 2:
            if (year % 4 == 0 && year % 400 != 0) {
                // just to be sure, nobody knows how long will be this app used
                if (day == 30) {
                    day = 1;
                    month++;
                }
            } else {
                if (day == 29) {
                    day = 1;
                    month++;
                }
            }
            break;
    }
}

/**
 * Get time string
 * @param time
 * @return time string
 */
std::string Cfg::get_time_str(uint64_t time) {
    std::stringstream out;

    if (time == 0xFFFFFFFFFF)
        return "UNKNOWN";

    int day, month, year;
    ConvertFromMJD(time >> 24, day, month, year);

    unsigned hours;
    unsigned minutes;
    unsigned seconds;

    if (local_time_offset_polarity) {
        hours   = ((time >> 16) - ((local_time_offset >> 8) & 0xFF)) & 0xFF;
        minutes = ((time >>  8) - ((local_time_offset >>  0) & 0xFF)) & 0xFF;
        seconds = ((time >>  0)) & 0xFF;
    } else {
        hours   = ((time >> 16) + ((local_time_offset >> 8) & 0xFF)) & 0xFF;
        minutes = ((time >>  8) + ((local_time_offset >>  0) & 0xFF)) & 0xFF;
        seconds = ((time >>  0)) & 0xFF;
    }


    unsigned up;
    unsigned down;
    bool carry = false;

    // minutes
    {
        up   = (minutes >> 4) & 0xF;
        down = (minutes >> 0) & 0xF;
        if (down >= 0xA) {
            up++;
            down = down - 0xA;
        }

        if (up >= 0x6) {
            up = up - 0x6;
            carry = true;
        }

        minutes = (up << 4) | down;
    }

    // hours
    {
        up   = (hours >> 4) & 0xF;
        down = (hours >> 0) & 0xF;

        if (carry) {
            down++;
            carry = false;
        }

        if (down >= 0xA) {
            up++;
            down = down - 0xA;
        }

        if (up >= 0x3) {
            up = up - 0x2;
            carry = true;
        }

        if (up == 0x2 && down >= 0x4) {
            up = 0x0;
            down = down - 0x4;
            carry = true;
        }

        hours = (up << 4) | down;
    }

    if (carry)
        incDay(day, year, month);

    out << std::dec << std::setw(2) << std::setfill('0') << year << "-"
        << std::dec << std::setw(2) << std::setfill('0') << month << "-"
        << std::dec << std::setw(2) << std::setfill('0') << day;

    out << " ";

    out << std::hex << std::setw(2) << std::setfill('0') << hours
        << ":"
        << std::hex << std::setw(2) << std::setfill('0') << minutes
        << ":"
        << std::hex << std::setw(2) << std::setfill('0') << seconds;

    return out.str();
}

/**
 * Get duration string
 * @param duration duration num
 * @return duration string
 */
std::string Cfg::get_duration_str(uint64_t duration) {
    std::stringstream out;

    if (duration == 0xFFFFFF)
        return "UNKNOWN";

    out << std::hex << std::setw(2) << std::setfill('0') << (unsigned) ((duration >> 16) & 0xFF)
        << ":"
        << std::hex << std::setw(2) << std::setfill('0') << (unsigned) ((duration >> 8) & 0xFF)
        << ":"
        << std::hex << std::setw(2) << std::setfill('0') << (unsigned) ((duration >> 0) & 0xFF);

    return out.str();
}

/**
 * Get dirname for program
 * @param program_id program id of program to create dirname for
 * @return dirname string
 */
std::string Cfg::get_dir_name(unsigned program_id) {
    std::stringstream dirname;

    for (auto it = data.begin(); it != data.end(); ++it) {

        if ((*it).first == program_id) {
            dirname.str("");
            dirname << "0x";
            for (auto jt = programs.begin(); jt != programs.end(); ++jt) {
                if ((*jt).first == (*it).first) {
                    dirname  << std::hex << std::setw(4) << std::setfill('0') << (*jt).second;
                }
            }

            dirname << "-";
            for (unsigned i = 0; i < (*it).second->provider.size(); ++i)
                if (isprint((*it).second->provider[i]) && (*it).second->provider[i] != '/')
                    dirname << (*it).second->provider[i];
                else
                    dirname << "-";

            dirname << "-";
            for (unsigned i = 0; i < (*it).second->name.size(); ++i)
                if (isprint((*it).second->name[i]) && (*it).second->name[i] != '/')
                    dirname << (*it).second->name[i];

            return dirname.str();

        }
    }
}

/**
 * Create directory structure and open video descriptors
 * @return true on success
 */
bool Cfg::create_dirs() {
    std::string dirname;
    for (auto it = data.begin(); it != data.end(); ++it) {
        dirname = m_dir_name + "/" + get_dir_name((*it).first);

        if (mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
            ERROR << m_dir_name << ":"
                  << strerror(errno) << std::endl;
            TSDBG << std::endl;
            return false;
        }

        std::string fname = dirname + "/video.m2v";
        (*it).second->video.open(fname, std::ios::binary);
        if (! (*it).second->video.is_open()) {
            ERROR << fname << ":"
                     << strerror(errno) << std::endl;
            TSDBG << std::endl;
            return false;
        }
    }

    TSDBG << "Directory structure created\n";
    return true;
}

/**
 * Flush all data to dirs
 * @return true on success
 */
bool Cfg::flush() {
    std::string dirname;

    for (auto it = data.begin(); it != data.end(); ++it) {
        dirname = m_dir_name + "/" + get_dir_name((*it).first);

        std::ofstream out;

        std::string fname = dirname + "/epg-present.txt";
        out.open(fname);
        if (! out.is_open()) {
            ERROR << fname << ":"
                     << strerror(errno) << std::endl;
            TSDBG << std::endl;
            return false;
        }

        // sort items!
        (*it).second->present.sort(less_first<ServiceInfo::item_t>());
        (*it).second->scheduled.sort(less_first<ServiceInfo::item_t>());

        for (auto jt = (*it).second->present.begin(); jt != (*it).second->present.end(); ++jt) {
            out << get_time_str((*jt).second.first.first);
            out << " - ";
            for (int i = 0; i < (*jt).second.second.size(); ++i)
                if ((char)(*jt).second.second[i] < (char)0xc0 || (char)(*jt).second.second[i] > (char)0xcf) // punctuation
                    out << (*jt).second.second[i];      // desc
            out << " - (" << get_duration_str((*jt).second.first.second) << ")\n"; // duration
        }
        out.close();

        fname = dirname + "/epg-schedule.txt";
        out.open(fname);
        if (! out.is_open()) {
            ERROR << fname << ":"
                     << strerror(errno) << std::endl;
            TSDBG << std::endl;
            return false;
        }

        for (auto jt = (*it).second->scheduled.begin(); jt != (*it).second->scheduled.end(); ++jt) {
            out << get_time_str((*jt).second.first.first);
            out << " - ";
            for (int i = 0; i < (*jt).second.second.size(); ++i)
                if ((char)(*jt).second.second[i] < (char)0xc0 || (char)(*jt).second.second[i] > (char)0xcf) // punctuation
                    out << (*jt).second.second[i];      // desc
            out << " - (" << get_duration_str((*jt).second.first.second) << ")\n"; // duration
        }
        out.close();

        // close video stream
        (*it).second->video.close();

        fname = dirname + "/audio.wav";

        if ((*it).second->audio.str().size())
            write_wav(fname.c_str(), (*it).second->audio.str().c_str(), (*it).second->audio.str().size());
        else
            WARN << "No data for " << fname << std::endl;

        out.close();
    }

    return true;
}

static int _kbhit() {
    int r;
    fd_set rfds;
    struct timeval tv;
    struct termios term,oterm;
    tcgetattr(0,&oterm);
    memcpy(&term,&oterm,sizeof(term));
    cfmakeraw(&term);
    tcsetattr(0,TCSANOW,&term);
    FD_ZERO(&rfds);
    FD_SET(0,&rfds);
    tv.tv_sec=tv.tv_usec=0;
    r=select(1,&rfds,NULL,NULL,&tv);
    tcsetattr(0,TCSANOW,&oterm);
    return r;
}

/**
 * Convert mem to wav file
 * http://read.pudn.com/downloads146/doc/635603/c/writewav/writewav.c__.htm
 * http://www.un4seen.com/doc/#bass/BASS_ErrorGetCode.html
 * @param fname output filename
 * @param audio mem audio
 * @param size size of audio
 * @return true on success
 */
bool Cfg::write_wav(const char * fname, const char * audio, size_t size) {
    const int BUF_SIZE = 8192;
    BASS_CHANNELINFO info;
    DWORD chan,p;
    QWORD pos;
    FILE *fp;
    short * buf;
    WAVEFORMATEX wf;

    buf = new short[BUF_SIZE];

    // not playing anything, so don't need an update thread
    BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD,0);

    // setup output - "no sound" device, 44100hz, stereo, 16 bits
    if (!BASS_Init(0,44100,0,0,NULL)) {
        ERROR << "BASS_Init\n";
        ERROR << BASS_ErrorGetCode() << std::endl;
        return false;
    }

    if ((chan=BASS_StreamCreateFile(true, audio, 0, size, BASS_STREAM_DECODE))) {
        pos=BASS_ChannelGetLength(chan,BASS_POS_BYTE);
    } else {
        ERROR << "BASS_StreamCreateFile " << fname << "(" << size << ")\n";
        ERROR << BASS_ErrorGetCode() << std::endl;
        return false;
    }

    if (!(fp=fopen(fname,"wb"))) {
        ERROR << "Can't create file\n";
        return false;
    }

    TSDBG << "writing to BASS.WAV file...\n";

    // write WAV header
    BASS_ChannelGetInfo(chan,&info);
    wf.wFormatTag=1;
    wf.nChannels=info.chans;
    wf.wBitsPerSample=(info.flags&BASS_SAMPLE_8BITS?8:16);
    wf.nBlockAlign=wf.nChannels*wf.wBitsPerSample/8;
    wf.nSamplesPerSec=info.freq;
    wf.nAvgBytesPerSec=wf.nSamplesPerSec*wf.nBlockAlign;

    fwrite("RIFF\0\0\0\0WAVEfmt \20\0\0\0",20,1,fp);
    fwrite(&wf,16,1,fp);
    fwrite("data\0\0\0\0",8,1,fp);

    while (!_kbhit() && BASS_ChannelIsActive(chan)) {
        int c=BASS_ChannelGetData(chan,buf,BUF_SIZE);
        fwrite(buf,1,c,fp);
        pos=BASS_ChannelGetPosition(chan,BASS_POS_BYTE);
    }

    // complete WAV header
    fflush(fp);
    p=ftell(fp);
    fseek(fp,4,SEEK_SET);
    putw(p-8, fp);
    fflush(fp);
    fseek(fp,40,SEEK_SET);
    putw(p-44,fp);
    fflush(fp);
    fclose(fp);

    BASS_Free();

    return true;
}

