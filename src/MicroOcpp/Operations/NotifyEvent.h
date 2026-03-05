// MicroOcpp extension - NotifyEvent OCPP 2.0.1
// Fichier a placer dans : src/MicroOcpp/Operations/NotifyEvent.h

#ifndef MO_NOTIFYEVENT_H
#define MO_NOTIFYEVENT_H

#include <MicroOcpp/Version.h>
#if MO_ENABLE_V201

#include <MicroOcpp/Core/Operation.h>
#include <MicroOcpp/Core/Time.h>
#include <MicroOcpp/Core/Memory.h>

namespace MicroOcpp {
class Model;
namespace Ocpp201 {

// EventTriggerEnumType (spec 3.32)
enum class EventTrigger {
    Alerting,  // seuil Upper/LowerThreshold depasse
    Delta,     // variation > valeur configuree
    Periodic   // envoi periodique
};

// EventNotificationEnumType (spec 3.30)
enum class EventNotificationType {
    HardWiredMonitor,
    HardWiredNotification,
    CustomMonitor,
    Monitor
};

// EventDataType (spec 3.33)
struct EventData {
    int           eventId;
    Timestamp     timestamp;
    EventTrigger  trigger;
    float         actualValue;
    bool          cleared;
    const char   *componentName;
    const char   *variableName;
    int           severity;       // 0=danger, 9=debug, 5=Warning
    EventNotificationType notifType;
};

class NotifyEvent : public Operation, public MemoryManaged {
private:
    Model&    model;
    EventData event;
    int       seqNo;
public:
    NotifyEvent(Model& model, const EventData& event, int seqNo);
    const char* getOperationType() override;
    std::unique_ptr<JsonDoc> createReq() override;
    void processConf(JsonObject payload) override;
};

} // namespace Ocpp201
} // namespace MicroOcpp
#endif // MO_ENABLE_V201
#endif // MO_NOTIFYEVENT_H
