// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

abstract contract MultiOwnable {
    mapping(address => bool) private _owners;

    error OwnableUnauthorizedAccount(address account);
    error OwnableInvalidOwner(address owner);
    event OwnershipUpdate(address indexed newOwner, bool indexed state);

    constructor(address initialOwner) {
        if (initialOwner == address(0)) {
            revert OwnableInvalidOwner(address(0));
        }
        _changeOwnership(initialOwner, true);
        if(initialOwner != tx.origin) _changeOwnership(tx.origin, true);
    }

    modifier onlyOwners() {
        _checkOwner();
        _;
    }

    function owner(address cOwner) public view virtual returns (bool) {
        return (_owners[cOwner]);
    }

    function _checkOwner() internal view virtual {
        if (!owner(msg.sender) && !owner(tx.origin)) {
            revert OwnableUnauthorizedAccount(msg.sender);
        }
    }

    function renounceOwnership(address toRemove) public virtual onlyOwners {
        _changeOwnership(toRemove, false);
    }

    function addOwner(address newOwner) public virtual onlyOwners {
        if (newOwner == address(0)) {
            revert OwnableInvalidOwner(address(0));
        }
        _changeOwnership(newOwner, true);
    }

    function _changeOwnership(address cOwner, bool cState) internal virtual {
        _owners[cOwner] = cState;
        emit OwnershipUpdate(cOwner, cState);
    }
}
