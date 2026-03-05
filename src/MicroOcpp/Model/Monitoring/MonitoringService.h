// MicroOcpp extension - MonitoringService
// Fichier : src/MicroOcpp/Model/Monitoring/MonitoringService.h
#ifndef MO_MONITORINGSERVICE_H
#define MO_MONITORINGSERVICE_H
#include <MicroOcpp/Version.h>
#if MO_ENABLE_V201
#include <functional>
#include <vector>
#include <MicroOcpp/Operations/NotifyEvent.h>
#include <MicroOcpp/Core/Time.h>
namespace MicroOcpp {
class Context;
struct ThresholdMonitor {
    const char *componentName;
    const char *variableName;
    std::function<float()> getter;
    float threshold;
    int   severity;
    bool  alertActive  = false;
    unsigned long lastSentMs = 0;
};
class MonitoringService {
private:
    Context& context;
    std::vector<ThresholdMonitor> monitors;
    uint32_t  nextEventId = 0;       // 0 = non initialisé
    bool      eventIdReady = false;  // vrai après sync horloge OCPP
    int       seqNo = 0;
    static const unsigned long RESEND_MS = 30000;
    bool tryInitEventId();           // retourne true si l'horloge est synchro
    void sendNotifyEvent(ThresholdMonitor& mon, float value, bool cleared);
public:
    explicit MonitoringService(Context& context);
    void addUpperThreshold(const char *comp, const char *var,
        std::function<float()> getter, float threshold, int severity = 5);
    void loop();
};
} // namespace MicroOcpp
#endif // MO_ENABLE_V201
#endif // MO_MONITORINGSERVICE_H
