import sys
from thread import start_new_thread
import SimpleHTTPServer
import SocketServer
import paho.mqtt.client as mqtt
import socket

my_port = 8000

def startServer(the_file):
  print("serving file " + the_file)
  handler = SimpleHTTPServer.SimpleHTTPRequestHandler
  httpd = SocketServer.TCPServer(("", my_port), handler)
  #httpd.serve_forever()
  #just handle one request
  httpd.handle_request()

def getOwnIp():
  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  s.connect(("fritz.box",80))
  my_ip = s.getsockname()[0]
  s.close()
  return my_ip

def getOtaTopic():
  with open("src/config.h") as file:
    for line in file:
      if line.startswith("#define IN_TOPIC_OTA"):
        clientId = line.split()[2]
        return clientId.replace('"','')

def getMqttServer():
  with open("src/config.h") as file:
    for line in file:
      if line.startswith("#define MQTT_SERVER"):
        mqttServer = line.split()[2]
        return mqttServer.replace('"','')

mqttc = mqtt.Client( protocol='MQTTv31')
#mqttc.connect("192.168.2.3", 1883, 60)
mqttc.connect(getMqttServer(), 1883, 60)

print(sys.argv)
for idx,element in enumerate(sys.argv):
  if element == "-f":
    the_file = sys.argv[idx + 1]
    #mqttc.publish("clock/in/ota", my_ip + "," + my_port + ",/" + the_file)
    mqttc.publish(getOtaTopic(), "http://" + getOwnIp() + ":" + str(my_port) + "/" + the_file)
    startServer(the_file)
    #start_new_thread(the_file)
    print(the_file)

#print(sys.argv)
