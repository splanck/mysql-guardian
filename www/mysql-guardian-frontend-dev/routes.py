from flask import Flask, render_template, request, session, redirect, url_for
from models import db, User
from forms import SignupForm, Loginform

app = Flask(__name__)

app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql://alistair:W@rdyIT01@localhost/my_test_database'
db.init_app(app)

#this protects against xss 
app.secret_key = "development-key"

@app.route("/")
def index():
  return render_template("index.html")

@app.route("/about")
def about():
  return render_template("about.html")

@app.route("/home")
def home():
  return render_template("home.html")

@app.route("/signup", methods = ['GET', 'POST'])
def signup():
  form = SignupForm()

  if request.method == "POST":
    if form.validate() == False:
      return render_template('signup.html', form=form)
    else:
      newuser = User(form.first_name.data, form.last_name.data, form.email.data, form.password.data)
      db.session.add(newuser)
      db.session.commit()

      session['email'] = newuser.email
      return redirect(url_for('home'))

  elif request.method == "GET":
    return render_template("signup.html", form=form)

@app.route("/login", methods=["GET", "POST"])
def login():
  form = Loginform()

  if request.method == "POST":
    if form.validate() == "False":
      return render_template("login.html", form=form)
    else:
      email = form.email.data
      password = form.password.data
      user = User.query.filter_by(email=email).first()

      if user is not None and user.check_password(password):
        session['email'] = form.email.data
        return redirect(url_for('home'))
      else:
        return redirect(url_for('login'))

  elif request.method == 'GET':
    return render_template('login.html', form=form)

if __name__ == "__main__":
  app.run(debug=True)
