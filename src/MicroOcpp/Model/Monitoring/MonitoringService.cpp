// MicroOcpp extension - MonitoringService
// Fichier : src/MicroOcpp/Model/Monitoring/MonitoringService.cpp
#include <MicroOcpp/Version.h>
#if MO_ENABLE_V201
#include <MicroOcpp/Model/Monitoring/MonitoringService.h>
#include <MicroOcpp/Core/Context.h>
#include <MicroOcpp/Core/Request.h>
#include <MicroOcpp/Debug.h>
#include <Arduino.h>
using namespace MicroOcpp;

MonitoringService::MonitoringService(Context& context) : context(context) {}

bool MonitoringService::tryInitEventId() {
    if (eventIdReady) return true;
    // Timestamp() = 1970-01-01 00:00:00 (epoch UNIX)
    // On considère l'horloge synchronisée si elle dépasse 2020-01-01
    static const Timestamp minValidTime(2020, 0, 0, 0, 0, 0);
    const Timestamp& now = context.getModel().getClock().now();
    if (now < minValidTime) {
        MO_DBG_WARN("MonitoringService: horloge OCPP non synchronisee, eventId en attente");
        return false;
    }
    // Secondes depuis l'epoch Unix => eventId de départ unique après chaque reboot
    Timestamp epoch;
    nextEventId   = (uint32_t)(now - epoch);
    eventIdReady  = true;
    MO_DBG_INFO("MonitoringService: eventId initialise depuis horloge OCPP = %u", nextEventId);
    return true;
}

void MonitoringService::addUpperThreshold(
        const char *comp, const char *var,
        std::function<float()> getter, float threshold, int severity) {
    ThresholdMonitor m;
    m.componentName = comp;
    m.variableName  = var;
    m.getter        = getter;
    m.threshold     = threshold;
    m.severity      = severity;
    m.alertActive   = false;
    m.lastSentMs    = 0;
    monitors.push_back(m);
    MO_DBG_INFO("Monitor added: %s/%s > %.2f (sev=%d)", comp, var, threshold, severity);
}

void MonitoringService::sendNotifyEvent(
        ThresholdMonitor& mon, float value, bool cleared) {
    if (!tryInitEventId()) return; // horloge pas encore synchro
    Ocpp201::EventData ed;
    ed.eventId       = nextEventId++;
    ed.timestamp     = context.getModel().getClock().now();
    ed.trigger       = Ocpp201::EventTrigger::Alerting;
    ed.actualValue   = value;
    ed.cleared       = cleared;
    ed.componentName = mon.componentName;
    ed.variableName  = mon.variableName;
    ed.severity      = mon.severity;
    ed.notifType     = Ocpp201::EventNotificationType::HardWiredMonitor;

    auto op  = std::unique_ptr<Ocpp201::NotifyEvent>(
        new Ocpp201::NotifyEvent(context.getModel(), ed, seqNo++));
    auto req = makeRequest(std::move(op));
    context.initiateRequest(std::move(req));

    Serial.printf("[Monitor] NotifyEvent %s/%s=%.2f cleared=%s",        mon.componentName, mon.variableName,        value, cleared ? "true" : "false");
}

void MonitoringService::loop() {
    unsigned long now = millis();
    for (auto& mon : monitors) {
        float val = mon.getter();
        bool  over = (val > mon.threshold);
        if (over && !mon.alertActive) {
            // Montee : premier depassement
            mon.alertActive = true;
            mon.lastSentMs  = now;
            sendNotifyEvent(mon, val, false);
        } else if (over && mon.alertActive &&
                   (now - mon.lastSentMs > RESEND_MS)) {
            // Maintenu : re-alerte periodique
            mon.lastSentMs = now;
            sendNotifyEvent(mon, val, false);
        } else if (!over && mon.alertActive) {
            // Retour a la normale
            mon.alertActive = false;
            sendNotifyEvent(mon, val, true);
        }
    }
}
#endif // MO_ENABLE_V201
