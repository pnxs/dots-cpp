#include "ResourceUsage.h"
#include <sys/resource.h>
#include <dots/eventloop/seconds.h>

dots::ResourceUsage::ResourceUsage()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    setUserCpuTime(usage.ru_utime);
    setSystemCpuTime(usage.ru_stime);

    setMaxRss(usage.ru_maxrss);
    setMinorFaults(usage.ru_minflt);
    setMajorFaults(usage.ru_majflt);
    setNrSwaps(usage.ru_nswap);
    setInBlock(usage.ru_inblock);
    setOutBlock(usage.ru_oublock);
    setNrSignals(usage.ru_nsignals);
    setNrVoluntaryContextSwitches(usage.ru_nvcsw);
    setNrInvoluntaryContextSwitches(usage.ru_nivcsw);
}
