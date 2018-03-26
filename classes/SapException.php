<?php

class SapException extends Exception
{
    /**
     * Get the key of this error
     *
     * @return string
     */
    public function getMessageKey() {}

    /**
     * Get the type of this error (MSGTY)
     *
     * @return string
     */
    public function getMessageType() {}

    /**
     * Get the id of this error (MSGID)
     *
     * @return string
     */
    public function getMessageId() {}

    /**
     * Get the number of this error (MSGNO)
     *
     * @return string
     */
    public function getMessageNumber() {}

    /**
     * Get variable 1 of this error (MSGV1)
     *
     * @return null|string
     */
    public function getMessageVar1() {}

    /**
     * Get variable 2 of this error (MSGV1)
     *
     * @return null|string
     */
    public function getMessageVar2() {}

    /**
     * Get variable 3 of this error (MSGV1)
     *
     * @return null|string
     */
    public function getMessageVar3() {}

    /**
     * Get variable 4 of this error (MSGV1)
     *
     * @return null|string
     */
    public function getMessageVar4() {}

    /**
     * Get the internal SDK function that caused this error
     *
     * @return null|string
     */
    public function getNwSdkFunction() {}
}