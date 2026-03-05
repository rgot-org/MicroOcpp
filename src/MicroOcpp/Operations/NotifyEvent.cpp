// MicroOcpp extension - NotifyEvent OCPP 2.0.1
// Fichier a placer dans : src/MicroOcpp/Operations/NotifyEvent.cpp

#include <MicroOcpp/Version.h>
#if MO_ENABLE_V201

#include <MicroOcpp/Operations/NotifyEvent.h>
#include <MicroOcpp/Model/Model.h>
#include <MicroOcpp/Core/Time.h>
#include <MicroOcpp/Debug.h>
#include <stdio.h>

using namespace MicroOcpp::Ocpp201;
using MicroOcpp::JsonDoc;

static const char* serializeTrigger(EventTrigger t) {
    switch (t) {
        case EventTrigger::Alerting:  return "Alerting";
        case EventTrigger::Delta:     return "Delta";
        case EventTrigger::Periodic:  return "Periodic";
    }
    return "Alerting";
}

static const char* serializeNotifType(EventNotificationType n) {
    switch (n) {
        case EventNotificationType::HardWiredMonitor:      return "HardWiredMonitor";
        case EventNotificationType::HardWiredNotification: return "HardWiredNotification";
        case EventNotificationType::CustomMonitor:         return "CustomMonitor";
        case EventNotificationType::Monitor:               return "Monitor";
    }
    return "HardWiredMonitor";
}

NotifyEvent::NotifyEvent(MicroOcpp::Model& model, const EventData& event, int seqNo)
    : MemoryManaged("v201.Operation.", "NotifyEvent"),
      model(model), event(event), seqNo(seqNo) {}

const char* NotifyEvent::getOperationType() {
    return "NotifyEvent";
}

std::unique_ptr<JsonDoc> NotifyEvent::createReq() {
    size_t capacity =
        JSON_OBJECT_SIZE(4)   +
        (JSONDATE_LENGTH + 1) +
        JSON_ARRAY_SIZE(1)    +
        JSON_OBJECT_SIZE(9)   +
        (JSONDATE_LENGTH + 1) +
        16                    +
        JSON_OBJECT_SIZE(1)   +
        JSON_OBJECT_SIZE(1);

    auto doc = makeJsonDoc(getMemoryTag(), capacity);
    JsonObject payload = doc->to<JsonObject>();

    char genAt[JSONDATE_LENGTH + 1];
    model.getClock().now().toJsonString(genAt, sizeof(genAt));
    payload["generatedAt"] = genAt;
    payload["tbc"]         = false;
    payload["seqNo"]       = seqNo;

    JsonArray arr = payload.createNestedArray("eventData");
    JsonObject ed = arr.createNestedObject();

    ed["eventId"] = event.eventId;

    char ts[JSONDATE_LENGTH + 1];
    event.timestamp.toJsonString(ts, sizeof(ts));
    ed["timestamp"]             = ts;
    ed["trigger"]               = serializeTrigger(event.trigger);
    ed["eventNotificationType"] = serializeNotifType(event.notifType);
    //ed["severity"]              = event.severity;

    if (event.cleared) {
        ed["cleared"] = true;
    }

    char val[16];
    snprintf(val, sizeof(val), "%.2f", event.actualValue);
    ed["actualValue"] = val;

    JsonObject comp = ed.createNestedObject("component");
    comp["name"] = event.componentName;

    JsonObject var = ed.createNestedObject("variable");
    var["name"] = event.variableName;

    MO_DBG_INFO("NotifyEvent seqNo=%d %s/%s=%.2f cleared=%s",
        seqNo, event.componentName, event.variableName,
        event.actualValue, event.cleared ? "true" : "false");

    return doc;
}

void NotifyEvent::processConf(JsonObject payload) {
    MO_DBG_DEBUG("NotifyEvent confirmed");
}

#endif // MO_ENABLE_V201
