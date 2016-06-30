/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#ifndef CFG_H_
#define CFG_H_

#include <map>
#include <utility>
#include <list>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cinttypes>

/**
 * Store service information
 */
struct ServiceInfo {
    // <present_id, <time, duration>, desc>
    typedef std::pair< unsigned, std::pair< std::pair<uint64_t, uint64_t>, std::string>>  item_t;
    typedef std::list<item_t> list_t;
    list_t present;
    std::list<unsigned> present_ids;

    list_t scheduled;
    std::list<unsigned> scheduled_ids;

    std::stringstream audio;
    std::ofstream video;
    bool video_started;

    std::string name;
    std::string provider;

    ServiceInfo() { video_started = false; }
};

/**
 * Store configuration
 */
class Cfg {
  public:
    std::ofstream info;
    // program number : PID
    std::list< std::pair<unsigned, unsigned> > programs;
    // service_id : struct
    std::map<unsigned, struct ServiceInfo *> data;
    // video pid : PID
    std::list< std::pair<unsigned, unsigned> > videos;
    // audio pid : PID
    std::list< std::pair<unsigned, unsigned> > audios;
    unsigned local_time_offset;
    unsigned local_time_offset_polarity;

    static Cfg & getInstance() { static Cfg singleton; return singleton; }

    bool filename(std::string & name);

    bool open_info();
    void close_info() { info.close(); }

    bool audios_find(unsigned pid);
    bool videos_find(unsigned pid);

    bool audios_find_pid(unsigned pid, unsigned * res = 0);
    bool videos_find_pid(unsigned pid, unsigned * res = 0);

    bool search_pat(unsigned pid);
    bool write_wav(const char * fname, const char * audio, size_t size);

    bool create_dirs();
    bool flush();
    std::string get_dir_name(unsigned program_id);
    void ConvertFromMJD(int nMJD, int &nDay, int &nMonth, int &nYear);
    std::string get_time_str(uint64_t time);
    std::string get_duration_str(uint64_t time);

  private:
    Cfg() { local_time_offset = local_time_offset_polarity = 0; }
    std::string m_dir_name;
};

#endif // CFG_H_
