from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

@app.route('/')
def main():
    return render_template('index.html')

@app.route('/getdata/', methods=['POST', 'GET'])
def getdata():
    return render_template('layout.html')

ButtonPressed = 1        
@app.route('/button', methods=["GET", "POST"])
def button():
    if request.method == "POST":
        return render_template("index.html", ButtonPressed = ButtonPressed)
    return render_template("button.html", ButtonPressed = ButtonPressed)

if __name__ == "__main__":
    app.run(debug=True)

""" mySQL functions below """

