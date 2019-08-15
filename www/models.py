from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash
db = SQLAlchemy()

class Guardian_servers(db.Model):
  __tablename__ = 'servers'
  id = db.Column(db.Integer, primary_key = True)
  hostname = db.Column(db.String(100))
  port = db.Column(db.Integer)
  username = db.Column(db.String(100))
  password = db.Column(db.String(100))

  def __init__(self, hostname, port, username, password):
    self.hostname = hostname
    self.port = port
    self.username = username
    self.password = password 

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

class Guardian_backup(db.Model):
  __tablename__ = 'backup_history'
  id = db.Column(db.Integer, primary_key = True)
  server_id = db.Column(db.Integer)
  time = db.Column(db.String(200))
  db_name = db.Column(db.String(100))
  filename = db.Column(db.String(100))

  def __init__(self, server_id, time, db_name, filename):
    self.server_id = server_id
    self.time = time
    self.db_name = db_name
    self.filename = filename

class Guardian_health_check(db.Model):
  __tablename__ = 'health_checks'
  time = db.Column(db.Time, primary_key = True)

  def __init__(self):
    self.time = time