# webserve


✅ Request Line → Contains METHOD PATH HTTP_VERSION.
✅ Headers → Provides extra information about the request.
✅ Body (Optional) → Contains data (used in POST, PUT).


## Full HTTP Request Example


POST /api/login HTTP/1.1
Host: www.example.com
User-Agent: Mozilla/5.0
Accept: application/json
Content-Type: application/json
Content-Length: 45

{
  "username": "john",
  "password": "12345"
}

🔸 This request does the following:
✔ POST → Sends data to /api/login.
✔ Headers → Specifies the request format and metadata.
✔ Body → Sends login credentials as JSON.