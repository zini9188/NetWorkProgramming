// MainView.java : Java Chatting Client �� �ٽɺκ�
// read keyboard --> write to network (Thread �� ó��)
// read network --> write to textArea

import java.awt.Canvas;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.border.EmptyBorder;

public class JavaUdpEchoClientView extends JFrame {
	private JPanel contentPane;
	private JTextField textField; // ���� �޼��� ���°�
	private String id;
	private InetAddress ip_addr;
	private int port;
	private Canvas canvas;
	JButton sendBtn; // ���۹�ư
	JTextArea textArea; // ���ŵ� �޼����� ��Ÿ�� ����
	private DatagramSocket udp_socket; // �������
	private InputStream is;
	private OutputStream os;
	private DataInputStream dis;
	private DataOutputStream dos;


	public JavaUdpEchoClientView(String id, String ip, int port)
	{
		this.id = id;
		try {
			this.ip_addr = InetAddress.getByName(ip);
		} catch (UnknownHostException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		this.port = port;
		InitScreen();
		textArea.append("Connecting: " + ip + " " + port + "\n");
		try {
			udp_socket = new DatagramSocket();
		} catch (IOException e) {
			textArea.append("���� ���� ����!!\n");
		}
		StartNetwork();
	}

	public void StartNetwork() { // ���� ���� �޼ҵ� ����κ�
		Thread th = new Thread(new Runnable() { // �����带 ������ �����κ��� �޼����� ����
			public void run() {
				byte[] bb = new byte[128];
				DatagramPacket udp_packet = new DatagramPacket(bb, bb.length);
				while (true) {
					for(int i=0; i<bb.length; i++)
					{
						bb[i] = 0;
					}
					try {
						udp_socket.receive(udp_packet); // packet ����
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					String msg = new String(bb);
					msg = msg.trim();
					textArea.append(msg + "\n");
					textArea.setCaretPosition(textArea.getText().length());
				} // while�� ��
			}// run�޼ҵ� ��
		});
		th.start();
	}

	public void send_Message(String str) { // ������ �޼����� ������ �޼ҵ�
		byte[] bb = new byte[128];
		bb = str.getBytes();
		DatagramPacket udp_packet = new DatagramPacket(bb, bb.length, ip_addr, port);	
		try {
			udp_socket.send(udp_packet);
		} catch (IOException e) {
			textArea.append("�޼��� �۽� ����!!\n");
		}
	}

	public void InitScreen() { // ȭ�鱸�� �޼ҵ�
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 288, 392);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		contentPane.setLayout(null);
		JScrollPane scrollPane = new JScrollPane();
		scrollPane.setBounds(0, 0, 272, 302);
		contentPane.add(scrollPane);
		textArea = new JTextArea();
		scrollPane.setViewportView(textArea);
		//textArea.setForeground(new Color(255,0,0));
		textArea.setDisabledTextColor(new Color(0,0,0));
		textField = new JTextField();
		textField.setBounds(0, 312, 155, 42);
		contentPane.add(textField);
		textField.setColumns(10);
		sendBtn = new JButton("��   ��");
		sendBtn.setBounds(161, 312, 111, 42);
		contentPane.add(sendBtn);
		textArea.setEnabled(false); // ����ڰ� �������ϰ� ���´�
		setVisible(true);
		Myaction action = new Myaction();
		sendBtn.addActionListener(action); // ����Ŭ������ �׼� �����ʸ� ��ӹ��� Ŭ������
		textField.addActionListener(action);
	}

	class Myaction implements ActionListener // ����Ŭ������ �׼� �̺�Ʈ ó�� Ŭ����
	{
		@Override
		public void actionPerformed(ActionEvent e) {
			// �׼� �̺�Ʈ�� sendBtn�϶� �Ǵ� textField ���� Enter key ġ��
			if (e.getSource() == sendBtn || e.getSource() == textField) 
			{
				String msg = null;
				msg = String.format("[%s] %s\n", id, textField.getText());
				send_Message(msg);
				textField.setText(""); // �޼����� ������ ���� �޼��� ����â�� ����.
				textField.requestFocus(); // �޼����� ������ Ŀ���� �ٽ� �ؽ�Ʈ �ʵ�� ��ġ��Ų��				
			}
		}
	}
}