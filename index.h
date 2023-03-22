const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
      p {
        margin-top: 2rem;
      }
      .header {
        border-bottom: 1px solid #343a40;
      }
      .switch-box {
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        background-color: #212529;
        border: 1px solid #343a40;
        max-width: 500px;
        height: 300px;
        margin: 0 auto;
        border-radius: 5px;
        gap: 2rem;
      }
      .button {
        border: 1px solid #4e575f;
        background-color: #34393e;
        border-radius: 5px;
        padding: 15px 30px;
        transition: 500ms ease-out;
      }
      .button:hover {
        color: #6c757d;
        box-shadow: 0 0 25px 1px #cacaca;
      }

      #ty {
        transform: scale(0);
      }
      #note {
        transform: scale(1);
      }
    </style>
  </head>
  <body class="h-[100vh] bg-[#191D21] text-[#D3D3D3] p-[30px] mt-[2rem]">
    <div class="h-full">
      <div class="header pb-[2rem] mx-auto w-full h-auto">
        <div class="flex text-center items-center justify-center">
          <h1 class="text-4xl mb-[.5rem] font-bold uppercase">FLUSHIT!&nbsp;</h1>
          <h1 class="text-[#6C757D] text-4xl mb-[.5rem] font-bold">
            Smart Toilet
          </h1>
        </div>
        <p class="mt-[1rem] italic text-center text-xl">
          ~ A remote for Smart Toilet ~
        </p>
      </div>

      <div class="content mt-[4rem] h-full">
        <p class="state text-center mb-[2rem] text-xl">
          <span id="state" class="text-[#2a9d8f]">%STATE%</span>
        </p>
        <div class="switch-box">
          <p id="note" class="text-xl">flush it here...</p>
          <p>
            <button id="button" class="button text-2xl font-bold">Flush</button>
          </p>
          <p id="ty" class="italic text-xl">Thank you!</p>
        </div>
      </div>
    </div>
  </body>
  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    window.addEventListener("load", onLoad);
    function initWebSocket() {
      console.log("Trying to open a WebSocket connection...");
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage; //
    }
    function onOpen(event) {
      console.log("Connection opened");
    }
    function onClose(event) {
      console.log("Connection closed");
      setTimeout(initWebSocket, 2000);
    }
    function onMessage(event) {
      var state;
      if (event.data == "1") {
        state = "Recently you've flushed the toilet";
      } else {
        state = "You haven't flushed the toilet in a while";
      }
      document.getElementById("state").innerHTML = state;
    }
    function onLoad(event) {
      initWebSocket();
      initButton();
    }
    function initButton() {
      const btn = document.getElementById("button");
      const note = document.getElementById("note");
      const ty = document.getElementById("ty");
      var mainColor = btn.style.color;
      console.log("here" + mainColor);
      var count = 0;
      btn.addEventListener("click", () => {
        count = count + 1;
        if(count==1){
           console.log(count)
        btn.style.background = "#c6c4c4";
        btn.style.color = "#6C757D";
        btn.style.border = "0px";
        btn.style.boxShadow = "0 0 25px 1px #cacaca";
        btn.innerHTML = "Flushed!";
        btn.style.transition = "500ms";
        note.style.transform = "scale(0)";
        note.style.transition = "300ms ease-out";
        ty.style.transform = "scale(1)";
        ty.style.transition = "300ms ease-in";
        websocket.send("toggle");
        btn.disabled = true;
        }
        setTimeout(() => {
          btn.style.background = "#34393e";
          btn.style.color = "white";
          btn.style.border = "1px";
          btn.style.boxShadow = "0 0 0px 0px #cacaca";
          note.style.transform = "scale(1)";
          ty.style.transform = "scale(0)";
          btn.innerHTML = "Flush";
          count=0;
          btn.disabled = false;
        }, 30000);
      });
    }
  </script>
</html>
)rawliteral";