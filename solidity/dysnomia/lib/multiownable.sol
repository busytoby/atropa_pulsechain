// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

abstract contract MultiOwnable {
    mapping(address => bool) private _owners;

    error OwnableUnauthorizedAccount(address origin, address account, address what);
    error OwnableInvalidOwner(address origin, address owner, address what);
    event OwnershipUpdate(address indexed newOwner, bool indexed state);

    constructor(address initialOwner) {
        if (initialOwner == address(0)) {
            revert OwnableInvalidOwner(tx.origin, address(0), address(this));
        }
        _changeOwnership(initialOwner, true);
        //if(initialOwner != tx.origin) _changeOwnership(tx.origin, true);
    }

    modifier onlyOwners() {
        _checkOwner();
        _;
    }

    function owner() external view virtual returns (address) {
        return address(this);
    }

    function owner(address cOwner) public view virtual returns (bool) {
        return (_owners[cOwner]);
    }

    function _checkOwner() internal view virtual {
        if (!owner(msg.sender) && !owner(tx.origin)) {
            revert OwnableUnauthorizedAccount(tx.origin, msg.sender, address(this));
        }
    }

    function renounceOwnership(address toRemove) public virtual onlyOwners {
        _changeOwnership(toRemove, false);
    }

    function addOwner(address newOwner) public virtual onlyOwners {
        if (newOwner == address(0)) {
            revert OwnableInvalidOwner(tx.origin, address(0), address(this));
        }
        _changeOwnership(newOwner, true);
    }

    function _changeOwnership(address cOwner, bool cState) internal virtual {
        _owners[cOwner] = cState;
        emit OwnershipUpdate(cOwner, cState);
    }
}
