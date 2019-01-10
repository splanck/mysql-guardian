from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash
db = SQLAlchemy()

class Guardian_user(db.Model):
  __tablename__ = 'users'
  id = db.Column(db.Integer, primary_key = True)
  username = db.Column(db.String(100))
  password = db.Column(db.String(100))
  admin = db.Column(db.Integer)
  email = db.Column(db.String(120))

  def __init__(self, username, email, password, admin):
    self.username = username 
    self.email = email
    self.password = password 
    self.admin = admin


