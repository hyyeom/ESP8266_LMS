function validate(){
	var password = document.getElementById("password").value;
	var param = "/login?pass=" + document.getElementById("password").value;
	if ( password == "5093"){
		var xhr = new XMLHttpRequest();
		xhr.onreadystatechange = function() {
			if (this.readyState == 4 && this.status == 200) {
				document.getElementById("demo").innerHTML = this.responseText;
			}
		};
		xhr.open("GET", param, true);
		xhr.send();
			window.location = "control.html?mkey=1qaz2wsx3edc4rfv5tgb6yhn7ujm8iklo90p[]"; // Redirecting to other page.
			return false;
		}
	else{
		alert("Wrong!");
	}
}