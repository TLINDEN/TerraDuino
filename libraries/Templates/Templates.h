P(index_title_1) = "Temperatur";
P(base_content_left) = 
  "</title>"
  "</head>"
  "<body>";
P(base_content_right) = 
  "<hr>"
  "<p>powered by TemplateDuino and Webduino</p>"
  "</body>"
  "</html>";
P(base_title_left) = 
  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
  "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"de-de\" lang=\"de-de\" dir=\"ltr\">"
  "<head><title>";
P(index_content_4) = "</ul>";
P(index_content_1) = "<ul>";

void tpl_index(WebServer &server, DATA_index data) {
  server.printP(base_title_left);

  server.printP(index_title_1);

  server.printP(base_content_left);

  server.printP(index_content_1);

  server << "<li>Temperatur: " << data.temp << "&deg; C</li>" << endl;

  server << "<li>Luftfeuchte: " << data.humidity << "%</li>" << endl;

  server.printP(index_content_4);

  server.printP(base_content_right);

}

