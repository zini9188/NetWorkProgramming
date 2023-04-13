// Java Chatting Server

import java.awt.EventQueue;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.border.EmptyBorder;
import javax.swing.SwingConstants;

public class JavaUdpEchoServer extends JFrame {
	private JPanel contentPane;
	private JTextField textField; // ����� PORT��ȣ �Է�
	private JButton Start; // ������ �����Ų ��ư
	JTextArea textArea; // Ŭ���̾�Ʈ �� ���� �޽��� ���
	private int Port; // ��Ʈ��ȣ
	private Vector vc = new Vector(); // ����� ����ڸ� ������ ����


	public static void main(String[] args)
	{	
		JavaUdpEchoServer frame = new JavaUdpEchoServer();
			frame.setVisible(true);			
	}

	public JavaUdpEchoServer() { 	
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 280, 400);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		contentPane.setLayout(null);

		JScrollPane js = new JScrollPane();				

		textArea = new JTextArea();
		textArea.setColumns(20);
		textArea.setRows(5);
		js.setBounds(0, 0, 264, 254);
		contentPane.add(js);
		js.setViewportView(textArea);

		textField = new JTextField();
		textField.setHorizontalAlignment(SwingConstants.CENTER);
		textField.setText("30000");
		textField.setBounds(98, 264, 154, 37);
		contentPane.add(textField);
		textField.setColumns(10);

		JLabel lblNewLabel = new JLabel("Port Number");
		lblNewLabel.setBounds(12, 264, 98, 37);
		contentPane.add(lblNewLabel);
		Start = new JButton("���� ����");
		
		Myaction action = new Myaction();
		Start.addActionListener(action); // ����Ŭ������ �׼� �����ʸ� ��ӹ��� Ŭ������
		textField.addActionListener(action);
		Start.setBounds(0, 325, 264, 37);
		contentPane.add(Start);
		textArea.setEditable(false); // textArea�� ����ڰ� ���� ���ϰԲ� ���´�.	
	}
	
	class Myaction implements ActionListener // ����Ŭ������ �׼� �̺�Ʈ ó�� Ŭ����
	{
		@Override
		public void actionPerformed(ActionEvent e) {
			// �׼� �̺�Ʈ�� sendBtn�϶� �Ǵ� textField ���� Enter key ġ��
			if (e.getSource() == Start || e.getSource() == textField) 
			{
				if (textField.getText().equals("") || textField.getText().length()==0)// textField�� ���� ������� ������
				{
					textField.setText("��Ʈ��ȣ�� �Է����ּ���");
					textField.requestFocus(); // ��Ŀ���� �ٽ� textField�� �־��ش�
				} 			
				else 
				{
					try
					{
						Port = Integer.parseInt(textField.getText()); // ���ڷ� �Է����� ������ ���� �߻� ��Ʈ�� ���� ����.		
						ServerStart(); // ����ڰ� ����ε� ��Ʈ��ȣ�� �־����� ���� ���������� �޼ҵ� ȣ��			
					}
					catch(Exception er)
					{
						//����ڰ� ���ڷ� �Է����� �ʾ����ÿ��� ���Է��� �䱸�Ѵ�
						textField.setText("���ڷ� �Է����ּ���");
						textField.requestFocus(); // ��Ŀ���� �ٽ� textField�� �־��ش�
					}	
				}// else �� ��
			}

		}
	}
	//@SuppressWarnings("resource")
	private void ServerStart() {
		Thread th = new Thread(new Runnable() { // ����� ������ ���� ������
			@SuppressWarnings("resource")
			@Override
			public void run() {
				//socket = new ServerSocket(Port); // ������ ��Ʈ ���ºκ�
				Start.setText("����������");
				Start.setEnabled(false); // ������ ���̻� �����Ű�� �� �ϰ� ���´�
				textField.setEnabled(false); // ���̻� ��Ʈ��ȣ ������ �ϰ� ���´�
				DatagramSocket udp_socket = null;
				try {
					udp_socket = new DatagramSocket(Port);
				} catch (SocketException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				byte[] bb = new byte[128];
				DatagramPacket udp_packet = new DatagramPacket(bb, bb.length);
				while (true){
						for(int i=0; i<bb.length; i++)
						{
							bb[i] = 0;
						}
						try {
							udp_socket.receive(udp_packet);
						} catch (IOException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
						String msgStr = new String(bb);
						textArea.append(udp_packet.getAddress() + ":" + udp_packet.getPort() + " " + msgStr+"\n");
						textArea.setCaretPosition(textArea.getText().length());
						try {
							//udp_packet = new DatagramPacket(buffer, 
							// 		buffer.length, udp_packet.getAddress(), 
							//		udp_packet.getPort());
							// echo back
							udp_socket.send(udp_packet);
						} catch (IOException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
				}
			}
		});
		th.start();
	}
	// ���� userinfoŬ������
}
