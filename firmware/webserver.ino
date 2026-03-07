// parses and processes webpages
// if the webpage has %SOMETHING% or %SOMETHINGELSE% it will replace those strings with the ones defined
String processor(const String& var) {
  if (var == "FIRMWARE") {
    return FIRMWARE_VERSION;
  }

  if (var == "FREEFLASH") {
    return humanReadableSize((LittleFS.totalBytes() - LittleFS.usedBytes()));
  }

  if (var == "USEDFLASH") {
    return humanReadableSize(LittleFS.usedBytes());
  }

  if (var == "TOTALFLASH") {
    return humanReadableSize(LittleFS.totalBytes());
  }
  
  if (var == "SLIDERVALUE"){
    return sliderValue;
  }
    return String();
}

// 新增函数：生成文件列表 HTML（提取出来，便于复用）
String generateFileList(int page, int perPage) {
  String html = "";
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  int totalFiles = 0;
  while (file) {
    totalFiles++;
    file.close();
    file = root.openNextFile();
  }
  root.close();

  int start = (page - 1) * perPage;
  int end = start + perPage;
  int current = 0;

  root = LittleFS.open("/");
  file = root.openNextFile();
  while (file) {
    if (current >= start && current < end) {
      String filename = file.name();
      html += "<div class=\"file-item\">";
      html += "<span class=\"file-name\">" + filename + "</span>";
      // 新增：图片预览（假设文件名以 .gif/.jpg/.png 结尾）
      if (filename.endsWith(".gif") || filename.endsWith(".jpg") || filename.endsWith(".png")) {
        html += "<img src=\"/file?name=" + filename + "&action=show\" style=\"width:80px; height:auto; margin-left:10px; margin-right: 10px;\">";
      }
      html += "<button class=\"btn file-btn\" onclick=\"downloadDeleteButton('" + filename + "', 'download')\">Download</button>";
      html += "<button class=\"btn file-btn\" onclick=\"downloadDeleteButton('" + filename + "', 'delete')\">Delete</button>";
      html += "<button class=\"btn file-btn\" onclick=\"downloadDeleteButton('" + filename + "', 'play')\">Play</button>";
      html += "</div>";
    }
    current++;
    file.close();
    file = root.openNextFile();
  }
  root.close();

  // 生成分页按钮（使用 onclick 翻页）
  int totalPages = (totalFiles + perPage - 1) / perPage;
  html += "<div class=\"pagination\">";
  if (page > 1) {
    html += "<button class=\"btn\" onclick=\"navigatePage(" + String(page - 1) + ")\">Previous</button>";
  }
  for (int p = 1; p <= totalPages; p++) {
    html += "<button class=\"btn" + String(p == page ? " active" : "") + "\" onclick=\"navigatePage(" + String(p) + ")\">" + String(p) + "</button>";
  }
  if (page < totalPages) {
    html += "<button class=\"btn\" onclick=\"navigatePage(" + String(page + 1) + ")\">Next</button>";
  }
  html += "</div>";

  return html;
}

void configureWebServer() {
  // configure web server

  // if url isn't found
  server->onNotFound(notFound);

  // run handleUpload function when any file is uploaded
  server->onFileUpload(handleUpload);

  // visiting this page will cause you to be logged out
  server->on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->requestAuthentication();
    request->send(401);
  });

  // presents a "you are now logged out webpage
  server->on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);
    request->send_P(401, "text/html", logout_html, processor);
  });

  server->on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      request->send_P(200, "text/html", index_html, processor);
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }

  });
  
  // /listfiles 和 /list 支持 perPage 参数
  server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest *request) {
    int perPage = request->hasParam("perPage") ? request->getParam("perPage")->value().toInt() : 4;
    if (perPage < 1) perPage = 4;
    request->send(200, "text/html", generateFileList(1, perPage));
  });

  server->on("/list", HTTP_GET, [](AsyncWebServerRequest *request) {
    int page = request->hasParam("page") ? request->getParam("page")->value().toInt() : 1;
    int perPage = request->hasParam("perPage") ? request->getParam("perPage")->value().toInt() : 4;
    if (perPage < 1) perPage = 4;
    if (page < 1) page = 1;
    request->send(200, "text/html", generateFileList(page, perPage));
  });
  
       server->on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
   String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = inputMessage;
      dma_display->setBrightness8(sliderValue.toInt());
    }
      else {
      inputMessage = "No message sent";
    }
    Serial.print(inputMessage);
    request->send(200, "text/plain", "OK");
    });

  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

    if (checkUserWebAuth(request)) {
      request->send(200, "text/html", reboot_html);
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      ESP.restart();
      shouldReboot = true;
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });

  server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest *request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
        logmessage += " Auth: Success";
        Serial.println(logmessage);
        request->send(200, "text/plain", listFiles(true, 1, maxGIFsPerPage)); // Explicitly pass page and pageSize
    } else {
        logmessage += " Auth: Failed";
        Serial.println(logmessage);
        return request->requestAuthentication();
    }
  });

  server->on("/list", HTTP_GET, [](AsyncWebServerRequest *request) {
    int page = 1; // Default to the first page
    if (request->hasParam("page")) {
        page = request->getParam("page")->value().toInt();
    }

    //Serial.printf("Requested page: %d\n", page); // Debug log

    String fileList = listFiles(true, page, maxGIFsPerPage); // Generate the table content
    request->send(200, "text/html", fileList);
});


  server->on("/setColor", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("r") && request->hasParam("g") && request->hasParam("b")) {
        colorR = request->getParam("r")->value().toInt();
        colorG = request->getParam("g")->value().toInt();
        colorB = request->getParam("b")->value().toInt();

        Serial.printf("Color updated: R=%d, G=%d, B=%d\n", colorR, colorG, colorB);

        request->send(200, "text/plain", "Color updated");
    } else {
        request->send(400, "text/plain", "Missing parameters");
    }
  });

  server->on("/toggleGIF", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state")) {
        String state = request->getParam("state")->value();
        gifEnabled = (state == "on"); // Update the gifEnabled variable
        Serial.printf("GIF playback state changed: %s\n", gifEnabled ? "ON" : "OFF");
        request->send(200, "text/plain", "GIF playback state updated");
        virtualDisp->clearScreen();
    } else {
        request->send(400, "text/plain", "Missing 'state' parameter");
    }
 });
 
 server->on("/toggleLoopGif", HTTP_GET, [](AsyncWebServerRequest *request) {
  if (request->hasParam("state")) {
      String state = request->getParam("state")->value();
      loopGifEnabled = (state == "on");
      Serial.printf("Loop GIF state updated to: %s\n", loopGifEnabled ? "ON" : "OFF");
      request->send(200, "text/plain", "Loop GIF state updated");
  } else {
      request->send(400, "text/plain", "Missing 'state' parameter");
  }
  });

 server->on("/toggleClock", HTTP_GET, [](AsyncWebServerRequest *request) {
  if (request->hasParam("state")) {
      String state = request->getParam("state")->value();
      clockEnabled = (state == "on");
      if (clockEnabled) {
          scrollTextEnabled = false; // Disable scrolling text if the clock is enabled
      }
      Serial.printf("Clock state changed: %s\n", clockEnabled ? "ON" : "OFF");
      request->send(200, "text/plain", "Clock state updated");
      virtualDisp->clearScreen();
  } else {
      request->send(400, "text/plain", "Missing 'state' parameter");
  }
});

server->on("/toggleScrollText", HTTP_GET, [](AsyncWebServerRequest *request) {
  if (request->hasParam("state")) {
      String state = request->getParam("state")->value();
      scrollTextEnabled = (state == "on");
      if (scrollTextEnabled) {
          clockEnabled = false; // Disable the clock if scrolling text is enabled
      }
      Serial.printf("Scrolling text state changed: %s\n", scrollTextEnabled ? "ON" : "OFF");
      request->send(200, "text/plain", "Scrolling text state updated");
      virtualDisp->clearScreen();
  } else {
      request->send(400, "text/plain", "Missing 'state' parameter");
  }
});

  server->on("/updateScrollText", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("text") && request->hasParam("fontSize") && request->hasParam("speed")) {
        scrollText = request->getParam("text")->value();
        scrollFontSize = request->getParam("fontSize")->value().toInt();
        scrollSpeed = request->getParam("speed")->value().toInt();
        Serial.printf("Scrolling text updated: '%s', Font size: %d, Speed: %d\n", scrollText.c_str(), scrollFontSize, scrollSpeed);
        request->send(200, "text/plain", "Scrolling text updated");
    } else {
        request->send(400, "text/plain", "Missing parameters");
    }
  });

  server->on("/file", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);

      if (request->hasParam("name") && request->hasParam("action")) {
        String fileName = "/"+String(request->getParam("name")->value() );
        const char *fileAction = request->getParam("action")->value().c_str();

        logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

        if (!LittleFS.exists(fileName)) {
          Serial.println(logmessage + " ERROR: file does not exist");
          request->send(400, "text/plain", "ERROR: file does not exist");
        } else {
          Serial.println(logmessage + " file exists");
         
          if (strcmp(fileAction, "download") == 0) {
            logmessage += " downloaded";
            request->send(LittleFS, fileName, "application/octet-stream");
          } else if (strcmp(fileAction, "delete") == 0) {
            logmessage += " deleted";
            LittleFS.remove(fileName);
            request->send(200, "text/plain", "Deleted File: " + String(fileName));
          }
          else if (strcmp(fileAction, "play") == 0) {
            requestedGifPath = fileName;
            gifFile = FILESYSTEM.open(fileName);
            logmessage += " opening";
            request->send(200, "text/plain", "Playing: " + String(fileName));
          }
           else if (strcmp(fileAction, "show") == 0) {
            logmessage += " previewing"; 
            delay(100);
            request->send(LittleFS, fileName, "image/gif");
           }
          else {
            logmessage += " ERROR: invalid action param supplied";
            request->send(400, "text/plain", "ERROR: invalid action param supplied");
          }
          Serial.println(logmessage);
        }
      } else {
        request->send(400, "text/plain", "ERROR: name and action params required");
      }
    } else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });
}

void notFound(AsyncWebServerRequest *request) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);
  request->send(404, "text/plain", "Not found");
}

// used by server.on functions to discern whether a user has the correct httpapitoken OR is authenticated by username and password
bool checkUserWebAuth(AsyncWebServerRequest * request) {
  bool isAuthenticated = false; // replace false with true if you want to disable authentication

  if (request->authenticate(config.httpuser.c_str(), config.httppassword.c_str())) {
    Serial.println("is authenticated via username and password");
    isAuthenticated = true;
  }
  return isAuthenticated;
}

// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  // make sure authenticated before allowing upload
  if (checkUserWebAuth(request)) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index) {
      logmessage = "Upload Start: " + String(filename);
      // open the file on first call and store the file handle in the request object
      request->_tempFile = LittleFS.open("/" + filename, "w");
      Serial.println(logmessage);
    }

    if (len) {
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
      logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
      Serial.println(logmessage);
    }

    if (final) {
      logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      // close the file handle as the upload is now done
      request->_tempFile.close();
      Serial.println(logmessage);
      request->redirect("/");
    }
  } else {
    Serial.println("Auth: Failed");
    return request->requestAuthentication();
  }
}
