from flask import Flask, redirect, render_template, url_for, \
    json, request, make_response
import requests

app = Flask(__name__)


def get_data():
    req = requests.get('https://eswapi.herokuapp.com/api/v1/eswdata')
    all = req.json()
    return all['eswdata']


@app.route('/live-data')
def live_data():
    get = get_data()
    # print(get)
    get = get[len(get)-1]
    print(get)
    data = [get['x'], get['y'], get['t']]
    response = make_response(json.dumps(data))
    response.content_type = 'application/json'
    return response


@app.route('/live-data-init')
def live_data_init():
    get = get_data()
    response = make_response(json.dumps(get))
    response.content_type = 'application/json'
    return response


@app.route('/')
def index():
    global logout
    logout = True
    return render_template("index.html", notexist=False)


@app.route('/user/<username>')
def user(username):
    print(username)
    return render_template('user_info.html', username=username)


@app.route('/main', methods=['POST', 'GET'])
def main():
    username = ''
    if request.method == 'POST':
        username = request.form['username']
    print(username)
    return redirect(url_for('user', username=username))


if __name__ == "__main__":
    app.run(debug=True)
