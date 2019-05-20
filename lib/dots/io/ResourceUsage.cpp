#include "ResourceUsage.h"
#include <sys/resource.h>
#include <dots/common/seconds.h>

dots::ResourceUsage::ResourceUsage()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    userCpuTime(usage.ru_utime);
    systemCpuTime(usage.ru_stime);

    maxRss(usage.ru_maxrss);
    minorFaults(usage.ru_minflt);
    majorFaults(usage.ru_majflt);
    nrSwaps(usage.ru_nswap);
    inBlock(usage.ru_inblock);
    outBlock(usage.ru_oublock);
    nrSignals(usage.ru_nsignals);
    nrVoluntaryContextSwitches(usage.ru_nvcsw);
    nrInvoluntaryContextSwitches(usage.ru_nivcsw);
}
