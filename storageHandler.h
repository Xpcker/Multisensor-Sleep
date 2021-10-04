#include "SPIFFS.h"
String fileContent = "";
void setupSPIFFS()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
    }
    File fileToWrite = SPIFFS.open("/test.txt", FILE_WRITE);
    if (!fileToWrite)
    {
        Serial.println("There was an error opening the file for writing");
        return;
    }

    if (fileToWrite.println(""))
    {
        Serial.println("File was initiated");
        ;
    }
    else
    {
        Serial.println("File write failed");
    }

    fileToWrite.close();
    //fileContent="";
}
void startFileWithBracket()
{

    File fileToWrite = SPIFFS.open("/test.txt", FILE_WRITE);
    if (!fileToWrite)
    {
        // Serial.println("There was an error opening the file for writing");
        return;
    }

    if (fileToWrite.println("{"))
    {
        //Serial.println("File was written");;
    }
    else
    {
        //Serial.println("File write failed");
    }

    fileToWrite.close();

    // fileContent+="{";
}
void appendToFile(String data)
{
    File fileToAppend = SPIFFS.open("/test.txt", FILE_APPEND);
    if (!fileToAppend)
    {
        //Serial.println("There was an error opening the file for appending");
        return;
    }
    String d = data + String(";");
    if (fileToAppend.println(d))
    {
        Serial.println("File content was appended");
    }
    else
    {
        Serial.println("File append failed");
    }
    fileToAppend.close();

    // String d=data+String(";");
    // fileContent+=d;
}

void clearFile()
{
    File fileToWrite = SPIFFS.open("/test.txt", FILE_WRITE);
    if (!fileToWrite)
    {
        // Serial.println("There was an error opening the file for writing");
        return;
    }

    if (fileToWrite.println("")) //empties the file content
    {
        Serial.println("File cleared");
        ;
    }
    else
    {
        //Serial.println("File write failed");
    }

    fileToWrite.close();

    // fileContent="";
}

String readFileContent()
{
    File fileToRead = SPIFFS.open("/test.txt");
    String content;
    if (!fileToRead)
    {
        Serial.println("Failed to open file for reading");
        //  return;
    }

    Serial.println("File Content:");

    while (fileToRead.available())
    {

        content = content + String((char)fileToRead.read());
    }

    fileToRead.close();
    return content;

    // return fileContent;
}
