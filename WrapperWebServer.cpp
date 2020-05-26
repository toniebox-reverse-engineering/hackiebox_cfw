#include "WrapperWebServer.h"
#include "Hackiebox.h"

#include <SLFS.h>

void WrapperWebServer::begin() {  
  _server = new WebServer(80);

  _server->enableCORS(true); //DEV ONLY!

  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  _server->onNotFound([&](){ WrapperWebServer::handleNotFound(); });
  _server->on("/", HTTP_GET, [&](){ WrapperWebServer::handleRoot(); });
  _server->on("/api/sse", HTTP_GET, [&](){ WrapperWebServer::handleSse(); });
  _server->on("/api/ajax", HTTP_GET, [&](){ WrapperWebServer::handleAjax(); });
  _server->on("/api/upload/file", HTTP_POST, [&](){ }, [&](){ WrapperWebServer::handleUploadFile(); });
  _server->on("/api/upload/flash-file", HTTP_POST, [&](){ WrapperWebServer::handleUploadFlashFile(); });
  _server->begin();
}
void WrapperWebServer::handle(void) {
  _server->handleClient();
}

void WrapperWebServer::handleNotFound(void) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += _server->uri();
  message += "\nMethod: ";
  message += (_server->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += _server->args();
  message += "\n";
  for (uint8_t i=0; i<_server->args(); i++){
    message += " " + _server->argName(i) + ": " + _server->arg(i) + "\n";
  }
  _server->send(404, "text/plain", message);
}

void WrapperWebServer::handleRoot(void) {
  Box.boxPower.feedSleepTimer();
  _server->send(200, "text/html", "ROOT");
}
void WrapperWebServer::handleSse(void) {
  Box.boxPower.feedSleepTimer();
  _server->send(200, "text/event-stream", "SSE");
  //TODO: Keep alive connection without blocking others
}
void WrapperWebServer::handleAjax(void) {
  Box.boxPower.feedSleepTimer();
  String cmd = _server->arg("cmd");

  if (cmd) {
    if (cmd.equals("get-config")) {
      _server->send(200, "text/json", Config.getAsJson());
      return;
    } else if (cmd.equals("get-dir")) {
      String path = _server->arg("path");
      if (!path)
        path = String();
      _server->send(200, "text/json", Box.boxSD.jsonListDir((char*)path.c_str()));
      return;
    } else if (cmd.equals("get-file")) {
      String filename = _server->arg("filepath");
      long read_start = _server->arg("start").toInt();
      long read_length = _server->arg("length").toInt();
      if (commandGetFile(&filename, read_start, read_length))
        return;
    } else if (cmd.equals("get-flash-file")) {
      String filename =_server->arg("filepath");
      long read_start = _server->arg("start").toInt();
      long read_length = _server->arg("length").toInt();
      if (commandGetFlashFile(&filename, read_start, read_length))
        return;
    } else if (cmd.equals("copy-file")) {
      char* source = (char*)_server->arg("source").c_str();
      char* target = (char*)_server->arg("target").c_str();
      //TBD
    } else if (cmd.equals("move-file")) {
      char* source = (char*)_server->arg("source").c_str();
      char* target = (char*)_server->arg("target").c_str();
      if (!FatFs.isDir(source) && !FatFs.exists(target)) {
        if (FatFs.rename(source, target)) {
          sendJsonSuccess();
          return;
        }
      }
    } else if (cmd.equals("delete-file")) {
      char* filepath = (char*)_server->arg("filepath").c_str();
      if (!FatFs.isDir(filepath) && FatFs.exists(filepath)) {
        if (FatFs.remove(filepath)) {
          sendJsonSuccess();
          return;
        }
      }
    } else if (cmd.equals("create-dir")) {
      char* dir = (char*)_server->arg("dir").c_str();
      if (!FatFs.exists(dir)) {
        if (FatFs.mkdir(dir)) {
          sendJsonSuccess();
          return;
        }
      }

    } else if (cmd.equals("move-dir")) {
      char* source = (char*)_server->arg("source").c_str();
      char* target = (char*)_server->arg("target").c_str();
      if (FatFs.isDir(source) && !FatFs.exists(target)) {
        if (FatFs.rename(source, target)) {
          sendJsonSuccess();
          return;
        }
      }
    } else if (cmd.equals("copy-dir")) {
      char* source = (char*)_server->arg("source").c_str();
      char* target = (char*)_server->arg("target").c_str();
      //TBD
    } else if (cmd.equals("delete-dir")) {
      char* dir = (char*)_server->arg("dir").c_str();
      if (FatFs.isDir(dir) && FatFs.exists(dir)) {
        if (FatFs.rmdir(dir)) {
          sendJsonSuccess();
          return;
        }
      }
    } else if (cmd.equals("box-power")) { 
      String sub = _server->arg("sub");
      if (sub.equals("reset")) {
        Box.boxPower.reset();
      } else if (sub.equals("hibernate")) {
        Box.boxPower.hibernate();
      }
    }
  }
  handleNotFound();
}

void WrapperWebServer::sendJsonSuccess() {
  _server->send(200, "text/json", "{ \"success\": true }");
}

bool WrapperWebServer::commandGetFile(String* path, long read_start, long read_length) {
  FileFs file;
  if (file.open((char*)path->c_str(), FA_OPEN_EXISTING | FA_READ)) {
    if (read_length == 0 || file.fileSize() < read_length) 
      read_length = file.fileSize();

    String filename = *path;
    int16_t index = filename.lastIndexOf("/");
    if (index != -1)
      filename.remove(0, index+1);

    _server->setContentLength(read_length);
    _server->sendHeader("Content-Disposition", (String("attachment; filename=\"") + filename + String("\"")).c_str());
    _server->send(200, "application/octet-stream", "");

    uint8_t buffer[512]; //higher buffer size may scramble the stream
    size_t read;
    size_t write;

    file.seekSet(read_start);
    while (file.curPosition() < read_length) {
      //_server->client().write(file.readChar());
      read = file.read(buffer, sizeof(buffer)); //TODO: may read to much if size is limited
      if (read == 0)
        break; //error or empty

      write = _server->client().write(buffer, read);
      if (write == 0) 
        break; //error*/
    }

    file.close();
    return true;
  } else {
    Log.error("Could not open %s", path);
  }
  return false;
}
bool WrapperWebServer::commandGetFlashFile(String* path, long read_start, long read_length) {
  if (SerFlash.open((char*)path->c_str(), FS_MODE_OPEN_READ) == SL_FS_OK) {
    if (read_length == 0 || SerFlash.size() < read_length) 
      read_length = SerFlash.size();

    String filename = *path;
    int16_t index = filename.lastIndexOf("/");
    if (index != -1)
      filename.remove(0, index+1);

    _server->setContentLength(read_length);
    _server->sendHeader("Content-Disposition", (String("attachment; filename=\"") + filename + String("\"")).c_str());
    _server->send(200, "application/octet-stream", "");

    uint8_t buffer[512];
    size_t read = -1;
    SerFlash.seek(read_start);
    while (SerFlash.available()) {
      read = SerFlash.readBytes(buffer, sizeof(buffer)); //TODO: may read to much if size is limited
      if (read == 0)
        break; //error or empty
      if (_server->client().write(buffer, read) == 0) 
        break; //error
    }

    SerFlash.close();
    return true;
  } else {
    Log.error("Could not open %s, error %s", path, SerFlash.lastErrorString());
  }
  return false;
}

void WrapperWebServer::handleUploadFile() {
  Box.boxPower.feedSleepTimer();
  HTTPUpload& upload = _server->upload();
  if (upload.status == UPLOAD_FILE_START) {
    char* filename = (char*)_server->arg("filepath").c_str();
    bool overwrite = false;
    if (!_server->arg("overwrite").equals(""))
      overwrite = true;
    long write_start = _server->arg("start").toInt();

    Log.info("handleUploadFile Name: %s, overwrite: %T, start=%l", filename, overwrite, write_start);

    uint8_t filemode = FA_CREATE_NEW | FA_WRITE;
    if (overwrite)
      filemode = FA_CREATE_ALWAYS | FA_WRITE;

    _uploadFileOpen = _uploadFile.open(filename, filemode);
    if (_uploadFileOpen) {
      _uploadFile.seekSet(write_start);
      return;
    }
    Log.error("File could not be opened.");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //Log.verbose("handleUploadFile Data: %i", upload.currentSize);
    if (_uploadFileOpen) {
      _uploadFile.write(upload.buf, upload.currentSize);
      return;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (_uploadFileOpen) {
      _uploadFile.close();
      Log.info("handleUploadFile Size: %ikB", upload.totalSize / 1024);
      sendJsonSuccess();
      return;
    }
  }
  handleNotFound();
}

void WrapperWebServer::handleUploadFlashFile() {
  Box.boxPower.feedSleepTimer();
  handleNotFound(); //TBD
}