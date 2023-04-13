package com.example.androidchat;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.text.Layout;
import android.text.method.ScrollingMovementMethod;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ScrollView;
import android.widget.TextView;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;

public class MainActivity extends AppCompatActivity {
    public EditText txtIpAddress, txtPortNumber, txtUserName, txtInput;
    public ScrollView scrollView;
    public TextView txtView;
    public Button btnLogin, btnAttach, btnSend;
    public Socket socket = null;
    public ObjectInputStream ois;
    public ObjectOutputStream oos;
    public boolean LoginStatus = false;
    public ImageView imgView;
    public MainActivity main;

    String UserName;
    String ip_addr = "10.0.2.2"; // Emulator PC의 127.0.0.1
    //final String ip_addr = "172.30.1.54"; // 실제 Phone으로 테스트 할 때는 이렇게 설정한다.
    String port_no = "30000";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        txtIpAddress = (EditText) findViewById(R.id.txtIpAddress);
        txtIpAddress.setText(ip_addr);
        txtPortNumber = (EditText) findViewById(R.id.txtPortNumber);
        txtPortNumber.setText(port_no);

        btnLogin = (Button) findViewById(R.id.btnLogin);
        btnAttach = (Button) findViewById(R.id.btnAttach);
        btnSend = (Button) findViewById(R.id.btnSend);
        txtUserName = (EditText) findViewById(R.id.txtUserName);
        txtInput = (EditText) findViewById(R.id.txtInput);
        txtView = (TextView) findViewById(R.id.txtView);
        txtView.setMovementMethod(new ScrollingMovementMethod());
        imgView = (ImageView) findViewById(R.id.imgView);

        txtUserName.setOnEditorActionListener(new TxtLoginAction());
        btnLogin.setOnClickListener( new BtnLoginClickListener());
        btnAttach.setOnClickListener(new BtnAttachClickListener());
        txtInput.setOnEditorActionListener(new TxtInputAction());
        btnSend.setOnClickListener(new BtnSendClickListener());

        main = this;
        //socket = new Socket();
    }

    // 문자열을 추가하고 자동 스크롤
    public void AppendText(String str) {
        String msg = str.trim();
        if(txtView != null){
            txtView.append(msg + "\n");
            final Layout layout = txtView.getLayout();
            if(layout != null){
                int scrollDelta = layout.getLineBottom(txtView.getLineCount() - 1)
                        - txtView.getScrollY() - txtView.getHeight();
                if(scrollDelta > 0)
                    txtView.scrollBy(0, scrollDelta);
            }
        }
    }

    // keyboard 사라지게 한다.
      private void hideKeyboard() {
        InputMethodManager inputManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        inputManager.hideSoftInputFromWindow(
                this.getCurrentFocus().getWindowToken(), 0);
    }

    // Login button 처리
    class BtnLoginClickListener implements View.OnClickListener {
        @RequiresApi(api = Build.VERSION_CODES.M)
        @Override
        public void onClick(View v) {

            if (LoginStatus == false) {
                //hideKeyboard();
                Login();
                 if (LoginStatus == false) // Login failed
                   return;
                txtUserName.setEnabled(false);
                btnLogin.setText("Logout");
            }
            else {
                txtUserName.setEnabled(true);
                btnLogin.setText("Login");
                Logout();
            }
        }
    }
    // Login button 처리
    class TxtLoginAction implements EditText.OnEditorActionListener {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
            if (LoginStatus == false) {
                hideKeyboard();
                Login();
                if (LoginStatus == false) // Login failed
                    return false;
                txtUserName.setEnabled(false);
                btnLogin.setText("Logout");
            }
            else {
                txtUserName.setEnabled(true);
                btnLogin.setText("Login");
                Logout();
            }
            return false;
        }
    }
    public void Login() {
        Thread th = new Thread() {
            public void run() {
                try {
                    AppendText("Login " + ip_addr + " " + port_no  + "\n");
                    ip_addr = txtIpAddress.getText().toString();
                    port_no = txtPortNumber.getText().toString();
                    //socket.setSoTimeout(1000);
                    SocketAddress addr = new InetSocketAddress(ip_addr, Integer.parseInt(port_no));
                    socket = new Socket();
                    socket.connect(addr, 1000);
                    //socket = new Socket(ip_addr, Integer.parseInt(port_no));
                    AppendText("Connected " + socket.toString() + "\n" );
                    if (!socket.isConnected()) {
                        AppendText("Login Failed\n");
                        return;
                    }
                    oos = new ObjectOutputStream(socket.getOutputStream());
                    //oos.flush();
                    ois = new ObjectInputStream(socket.getInputStream());
                    UserName = txtUserName.getText().toString();
                    ChatMsg obj = new ChatMsg(UserName,"100", "Hello");
                    SendChatMsg(obj);
                    LoginStatus = true;
                    DoReceive(); // Server에서 읽는 Thread 실행
                  } catch (IOException e) {
                    AppendText("Socket() connect failed\n");
                    e.printStackTrace();
                }
            }
        };
        th.start();
        try {
            th.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    // Image 표시
    public void ViewImageBytes(byte[] bytes){
        Bitmap bm = BitmapFactory.decodeByteArray(bytes,0, bytes.length);
        imgView.post(new Runnable()
        {
            public void run()
            {
                imgView.setImageBitmap(bm);
            }
        });
    }

    // Server Message 수신
    public void DoReceive() {
        new Thread() {
            public void run() {
               while (true && LoginStatus) {
                 ChatMsg cm;
                   cm = ReadChatMsg();
                   if (socket.isClosed())
                       return;
                   AppendText("["+cm.UserName+"] " + cm.data);
                   if (cm.code.equals("300")) { // image 첨부
                       ViewImageBytes(cm.imgbytes);
                   }
               }
            }
        }.start();
    }

    synchronized public void Logout() {
       new Thread() {
           public void run() {
               if (socket.isClosed())
                    return;
               AppendText("Logout"  + "\n");
               ChatMsg cm = new ChatMsg(UserName,"400", "Bye");
               //SendChatMsg(cm);
               try {
                   oos.writeObject(cm.code);
                   oos.writeObject(cm.UserName);
                   oos.writeObject(cm.data);
                   socket.close();

               } catch (IOException e) {
                   e.printStackTrace();
               }
                LoginStatus = false;
           }
        }.start();
    }

    // Attach button 처리, Image 첨부
    class BtnAttachClickListener implements View.OnClickListener {
        @Override
        public void onClick(View v) {
            // File 선택 Dialog 시작
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("image/*");
            startActivityForResult(intent,1); // Image 파일 선택하면 실행
        }
    }

    // Send Image File, Image 파일 선택하면 실행
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != 1 || resultCode != RESULT_OK) {
            return;
        }
        Uri dataUri = data.getData();
        BufferedInputStream bis;

        String filename = getFileName(dataUri);
        //AppendText(dataUri.toString());
        File file = new File(String.valueOf(dataUri));
        //AppendText(filename);
        byte[] imgbytes = null;
        InputStream in = null;
        try {
            in = getContentResolver().openInputStream(dataUri);
            Bitmap bm = BitmapFactory.decodeStream(in);
            ByteArrayOutputStream bytearray = new ByteArrayOutputStream();
            bm.compress(Bitmap.CompressFormat.JPEG, 100, bytearray);
            imgbytes = bytearray.toByteArray();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        //ViewImageBytes(imgbytes);
        ChatMsg cm = new ChatMsg(UserName, "300", filename);
        cm.imgbytes = imgbytes;
        SendChatMsg(cm);
    }
    // Uri 에서 filename 만 분리
    public String getFileName(Uri uri) {
        String result = null;
        if (uri.getScheme().equals("content")) {
            Cursor cursor = getContentResolver().query(uri, null, null, null, null);
            try {
                if (cursor != null && cursor.moveToFirst()) {
                    result = cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }
        }
        if (result == null) {
            result = uri.getLastPathSegment();
        }
        return result;
    }

    // Text 입력중 Enter key 처리
    class TxtInputAction implements EditText.OnEditorActionListener {
        @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
            String msg;
            msg = txtInput.getText().toString();
            if (msg.length()==0)
                return false;
            hideKeyboard(); // 호출하면 Software 키가 내려간다.
            ChatMsg cb = new ChatMsg(UserName, "200", msg);
            SendChatMsg(cb);
            txtInput.setText("");
            return false;
        }
    }

    // Text Send button 처리
    class BtnSendClickListener implements View.OnClickListener {
        @Override
        public void onClick(View v) {
            if (socket==null || socket.isClosed() || !socket.isConnected())
                return;

            String msg;
            msg = txtInput.getText().toString();
            if (msg.length()==0)
                return;
            hideKeyboard();
            ChatMsg cb = new ChatMsg(UserName, "200", msg);
            SendChatMsg(cb);
            txtInput.setText("");
        }
    }

    // SendChatMsg()
    public void SendChatMsg(ChatMsg cm)  {
        new Thread() {
            public void run() {
                // Java 호환성을 위해 각각의 Field를 따로따로 보낸다.
                try {
                    if (socket==null || socket.isClosed() || !socket.isConnected())
                        return;
                    oos.writeObject(cm.code);
                    oos.writeObject(cm.UserName);
                    oos.writeObject(cm.data);
                    if (cm.code.equals("300")) // Image 첨부인 경우
                        oos.writeObject(cm.imgbytes);
                    //oos.flush();
                    //notify(); // 다른 Thread를 wait에서 빠져 나오게 한다.
                } catch (IOException e) {
                    e.printStackTrace();
                }
             }
        }.start();
    }

    // ChatMsg 를 읽어서 Return, Java 호환성 문제로 field별로 수신해서 ChatMsg 로 만들어 Return
    public ChatMsg ReadChatMsg()  {
        String code = null, UserName = null, data = null;
        ChatMsg cm = new ChatMsg("","","");
        try {
            if (socket.isClosed())
                return cm;
            cm.code = (String) ois.readObject();
            cm.UserName = (String) ois.readObject();
            cm.data = (String) ois.readObject();
            if (cm.code.matches("300")) {
                cm.imgbytes = (byte[]) ois.readObject();
             }
             //AppendText(code + " " + UserName + " " + data + " " );

        } catch (ClassNotFoundException | IOException e) {
            e.printStackTrace();
            Logout();
        }
        return cm;
    }

}