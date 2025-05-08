#ifndef _PAINLESS_MESH_STOPWATCH_HPP
#define _PAINLESS_MESH_STOPWATCH_HPP

#include <arduino.h>

namespace painlessmesh
{
    class Stopwatch
    {
    public:
        static Stopwatch &getInstance()
        {
            static Stopwatch instance;
            return instance;
        }

        Stopwatch(const Stopwatch &) = delete;
        Stopwatch &operator=(const Stopwatch &) = delete;

        int timeSinceLastReportPearDataTask()
        {
            const unsigned long now = millis();
            if (lastReportPearDataTaskTimestamp == 0)
            {
                lastReportPearDataTaskTimestamp = now;
                return 0;
            }

            int timeSinceLastReportPearDataTask = now - lastReportPearDataTaskTimestamp;
            lastReportPearDataTaskTimestamp = now;
            return timeSinceLastReportPearDataTask;
        }

        int timeSinceLastRunPearTask()
        {
            const unsigned long now = millis();
            if (lastRunPearTaskTimestamp == 0)
            {
                lastRunPearTaskTimestamp = now;
                return 0;
            }

            int timeSinceLastRunPearTask = now - lastRunPearTaskTimestamp;
            lastRunPearTaskTimestamp = now;
            return timeSinceLastRunPearTask;
        }

        void reset()
        {
            lastReportPearDataTaskTimestamp = 0;
            lastRunPearTaskTimestamp = 0;
        }

    protected:
        Stopwatch(){}
        uint32_t lastReportPearDataTaskTimestamp = 0;
        uint32_t lastRunPearTaskTimestamp = 0;
    };
}

#endif