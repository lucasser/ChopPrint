#include "printer.h"

void Printer::loadConfig(JsonDocument& config) {
  setupAxis(config);
}

void Printer::setupAxis(JsonVariant config) {
  JsonArray axis(config["axis"]);
  for (JsonVariant i : axis) {
    char id = i["id"];
    AXIS.at(AXISBYID).loadConfig(i);
    activeAxis.push_back(id);
  }
  //[TODO]: setup heating stuff
}

Axis* Printer::getAxis(int id) {
  if (AXIS.at(id).init) {
    return &AXIS.at(id);
  }
  return nullptr;
}

Axis* Printer::getAxis(char id) {
  if (AXIS.at(AXISBYID).init) {
    return &AXIS.at(AXISBYID);
  }
  return nullptr;
}

void Printer::processCommand(String& in) {
  char command = in.charAt(0);
  if (in.length() < 3) {
    return;
  }
  if (command == 'l') {
    Serial.println("level");
    getAxis(in.charAt(2))->level();
  } else if (command == 't') {
    Serial.println("tool");
  } else if (command == 'u') {
    Serial.println("config updated");
  } else if (command == 's') {
    Serial.println("stopped");
    int j = 0;
    for (auto i : AXIS) {
      if (!i.init) {
        continue;
      }
      if (in.indexOf(axismap.find(j)->first) != -1 || in.indexOf('a') != -1) {
        i.suspend();
      }
      j++;
    }
  } else if (command == 'r') {
    Serial.println("moving");
  } else if (command == 'm') {
    moveCommand go = parseMove(in);
    Serial.println(go.toString());
    for (auto i : activeAxis) {
      int id = i;
      if (isnanf(go.coords.at(AXISBYID))) {
        getAxis(i)->delay(go.time);
      } else {
        getAxis(i)->generalMove({go.type, go.coords.at(AXISBYID), go.time});
      }
    }
  } else if (command == 'p') {
    Serial.println(toString());
  }
}

String Printer::toString() {
  String out = "axis: [{";
  for (auto i : activeAxis) {
    out += "\nid: ";
    out += i;
    out += "\naxis: ";
    char id = i;
    out += AXIS.at(AXISBYID).toString();
  }
  out += "}]";
  return out;
}

//[TODO?]: revisit and optimize logic
moveCommand Printer::parseMove(String& in) {
  moveCommand go; //where we moving next
  String temp = "";

  go.type = in.charAt(2);
  go.time = in.substring(in.indexOf('t') + 1, in.indexOf(' ', in.indexOf('t'))).toFloat();

  for (auto i : activeAxis) {
    int index = in.indexOf(i);
    if (index != -1) {
      temp = in.substring(index + 1, in.indexOf(' ', index));
    }
    int id = i;
    go.coords.at(AXISBYID) = temp.toFloat();
  }

  Serial.println(go.toString());
  return go;
}