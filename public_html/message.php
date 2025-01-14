<?php 

//debug
//echo client_check(path(1));
//echo current_datetime(); 
//echo NOW; 

function send_msg($arr_msg){
    header('Content-Type: application/json; charset=utf-8');
    
    $string_to_send = array (
        'ID' => '0815', // DB id of messages
        'topic' => 't.b.d.',
        'date' =>  date("Y.m.d"), 
        'message' => $arr_msg
    );
    print json_encode($string_to_send);
}

// set variable
$mac_addr = path(1);

// check if mac is missing
if (empty($mac_addr)) {
    $msg = array(
        'ID' = 0,
        'topic' = 'setup',
        'message' => array (
        'topic'	 => 'E409',
        'line1' => ' - E R R O R - ',
        'line2' => ' Missing Addr ',
        'line3' => ''
        )
    );
    send_msg($msg);
    exit;
}

// check if max is 12 char
if (strlen($mac_addr) !== 12 ){
    $msg = array(
        'ID' = 0,
        'topic' = 'setup',
        'message' => array (
            'topic'	 => 'E409',
            'line1' => ' - E R R O R - ',
            'line2' => substr($mac_addr, 0, 20),
            'line3' => 'is not a mac addr!'
        )
    );
    send_msg($msg);
    exit;
}

// check if mac is registered
if ( client_check($mac_addr) == 1 ) {
    $clientStatus = "known";
    send_msg(msg_select_latest());

} else {
    $clientStatus = "unknown";
    client_register($mac_addr);
    $replacements = array('line3'=> 'Your PassKey:' . client_get_passkey($mac_addr)['passkey'] );
    $msg = array_replace($msg_not_registered,$replacements);
    send_msg($msg);
}