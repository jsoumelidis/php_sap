<?php

class SapException extends Exception
{
    /**
     * Get the key of this error
     *
     * @return string
     */
    public function getMessageKey(): string {}

    /**
     * Get the type of this error (MSGTY)
     *
     * @return string
     */
    public function getMessageType(): string {}

    /**
     * Get the id of this error (MSGID)
     *
     * @return string
     */
    public function getMessageId(): string {}

    /**
     * Get the number of this error (MSGNO)
     *
     * @return string
     */
    public function getMessageNumber(): string {}

    /**
     * Get variable 1 of this error (MSGV1)
     *
     * @return null|string
     */
    public function getMessageVar1(): ?string {}

    /**
     * Get variable 2 of this error (MSGV1)
     *
     * @return null|string
     */
    public function getMessageVar2(): ?string {}

    /**
     * Get variable 3 of this error (MSGV1)
     *
     * @return null|string
     */
    public function getMessageVar3(): ?string {}

    /**
     * Get variable 4 of this error (MSGV1)
     *
     * @return null|string
     */
    public function getMessageVar4(): ?string {}

    /**
     * Get the internal SDK function that caused this error
     *
     * @return null|string
     */
    public function getNwSdkFunction(): ?string {}
}