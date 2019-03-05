from flask import Flask, render_template, request, session, redirect, url_for
from models import db, Guardian_user, Guardian_servers
from forms import SignupForm, Loginform
import random

app = Flask(__name__)

app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql+pymysql://alistair:W@rdyIT01@localhost/mysql_guardian'
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

@app.route("/guardian_users")
def guardian_users():
  get_guardian_users = Guardian_user.query.all()
  return render_template("guardian_users.html", get_guardian_users = get_guardian_users)

@app.route("/guardian_servers")
def guardian_servers():
  get_guardian_servers = Guardian_servers.query.all()
  return render_template("guardian_servers.html", get_guardian_servers = get_guardian_servers )

@app.route("/signup", methods = ['GET', 'POST'])
def signup():
  form = SignupForm()

  if request.method == "POST":
    if form.validate() == False:
      return render_template('signup.html', form=form)
    else:
      newuser = Guardian_user(form.username.data, form.email.data, form.password.data, admin = 1)
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
      user = Guardian_user.query.filter_by(email=email).first()

      if user is not None and user.check_password(password):
        session['email'] = form.email.data
        return redirect(url_for('home'))
      else:
        return redirect(url_for('login'))

  elif request.method == 'GET':
    return render_template('login.html', form=form)

if __name__ == "__main__":
  app.run(debug=True)