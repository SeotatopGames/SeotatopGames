using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerSpawn : MonoBehaviour
{
    GameObject player;

    LevelManager levelManager;

    private bool pullPlayer;

    private void Start()
    {
        levelManager = LevelManager.instance;
        player = GameObject.FindGameObjectWithTag("Player");
        PullPlayer();
    }

    private void Update()
    {
        if(pullPlayer)
        {
            PullPlayer();
        }
    }

    public void PullPlayer() //Player is in spawn or close enough -> Start Game
    {
        if (levelManager.playerInPlayerSpawn)
        {
            pullPlayer = false;
        }
        else
        {
            if (player != null)
            {
                player.GetComponent<CharacterController>().enabled = false;
                player.transform.SetPositionAndRotation(gameObject.transform.position, gameObject.transform.rotation);
                player.GetComponent<CharacterController>().enabled = true;
            }
            else
            {
                player = GameObject.FindGameObjectWithTag("Player");
            }
            pullPlayer = true;
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            PlayerInSpawn();
        }
    }

    //private void OnTriggerStay(Collider other)
    //{
    //    if (other.CompareTag("Player"))
    //    {
    //        PlayerInSpawn();
    //    }
    //}

    public void PlayerInSpawn()
    {
        levelManager.playerInPlayerSpawn = true;
        //levelManager.inElevator = false;
    }

    private void OnTriggerExit(Collider other)
    {
        PlayerLeftSpawn();
    }

    public void PlayerLeftSpawn()
    {
        levelManager.StartLevel();
        Destroy(this.gameObject);
    }
}
