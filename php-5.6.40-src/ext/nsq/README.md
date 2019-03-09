# php-nsq

NSQ  client for php7 ; <br/>


### intall :

    Dependencies: libevent

    1. sudo phpize
    2. ./configure 
    3. make  
    4. make install  

    add in your php.ini:

    extension = nsq.so;


### Example for pub:

```php
$nsqdAddr = array(
    "127.0.0.1:4150",
    "127.0.0.1:4154"
);

$nsq = new Nsq();
$isTrue = $nsq->connectNsqd($nsqdAddr);

for($i = 0; $i < 10000; $i++){
    $nsq->publish("test", "nihao");
}
$nsq->closeNsqdConnection();

// Deferred publish 
//function : deferredPublish(string topic,string message, int millisecond); 
//millisecond default : [0 < millisecond < 3600000]

$deferred = new Nsq();
$isTrue = $deferred->connectNsqd($nsqdAddr);
for($i = 0; $i < 20; $i++){
    $deferred->deferredPublish("test", "message daly", 3000); 
}
$deferred->closeNsqdConnection();

```

### Example for sub:
```php
<?php 

//sub

$nsq_lookupd = new NsqLookupd("127.0.0.1:4161"); //the nsqlookupd http addr
$nsq = new Nsq();
$config = array(
    "topic" => "test",
    "channel" => "struggle",
    "rdy" => 2,                //optional , default 1
    "connect_num" => 1,        //optional , default 1   
    "retry_delay_time" => 5000,  //optional, default 0 , if run callback failed, after 5000 msec, message will be retried
    "auto_finish" => true, //default true
);

$nsq->subscribe($nsq_lookupd, $config, function($msg,$bev){ 

    echo $msg->payload;
    echo $msg->attempts;
    echo $msg->message_id;
    echo $msg->timestamp;


});

```
### Nsq Object

* `connectNsqd($nsqdAddrArr)` <br/>
  publish use, You can also use it for health check;

* `closeNsqdConnection()` <br/>
  close connecNsqd's socket

* `publish($topic,$msg)` <br/>

* `deferredPublish($topic,$msg,$msec)` <br/>

* `subscribe($nsq_lookupd,$config,$callback)` <br/>

### Message Object

The following properties and methods are available on Message objects produced by a Reader
instance.

* `timestamp` <br/>
  Numeric timestamp for the Message provided by nsqd.
* `attempts` <br/>
  The number of attempts that have been made to process this message.
* `message_id` <br/>
  The opaque string id for the Message provided by nsqd.
* `payload` <br/>
  The message payload as a Buffer object.
* `finish($bev,$msg->message_id)` <br/>
  Finish the message as successful.
* `touch($bev,$msg->message_id)` <br/>
  Tell nsqd that you want extra time to process the message. It extends the
  soft timeout by the normal timeout amount.



### Tips :


1. `If you need some variable in callback ,you should use 'use' :` <br/>

```
$nsq->subscribe($nsq_lookupd, $config, function($msg,$bev) use ($you_variable){ 

    echo $msg->payload;
    echo $msg->attempts;
    echo $msg->message_id;
    echo $msg->timestamp;


});
```

2. `Requeue/Retry --  if you whant to retry your mesage when callback have something wrong, just throw any Exception , example:
` <br/>

```
<?php 

$nsq->subscribe($nsq_lookupd, $config, function($msg){ 
    try{
        echo $msg->payload . " " . "attempts:".$msg->attempts."\n";
        //do something
    }catch(Exception $e){

        if($msg->attempts < 3){
            //the message will be retried after you configure retry_delay_time 
            throw new Exception(""); 
        }else{
            echo $e->getMessage();
            return;
        }
    }

});
```


3. `If your have strong consuming ability ,you can add you rdy num and connect num` <br/>


4. `You can use supervisor to supervise process,The following configuration needs to be added to the supervisor configuration file: ` <br/>
```
    stopasgroup=true
    killasgroup=true
```

5. `If your execution time is more than 1 minute, you should use 'touch()' function ` <br/>
    



Changes
-------
* **2.4.0**
  * Fix pub bug
  * Fix sub coredump 
  * Fix touch bug
  * Add the waite,  when topic has no message
* **2.3.1**
  * Support the domain host of pub
  * Fix pub coredump 
* **2.3.0**
  * Optimized memory usage,  Guarantee stability of resident memory 
* **2.2.0**
  * Fix pub bug zend_mm_heap corrupted 
  * Fix pub block bug  when received the 'heartbeats' 
  * Add the bufferevent resource
  * Add the deferred publish
  * Add the touch function
  * Add the finish function
* **2.1.1**
  * Fix core dump
* **2.0**
  * retry
  * message object
  * fix c99 install error
  * license

QQ Group
--------
616063018
